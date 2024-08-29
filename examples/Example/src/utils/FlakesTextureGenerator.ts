import flakesTexture from './shaders/flakesTexture.wgsl';

const CIRCLE_POINTS = 10;
const BYTES_PER_PIXEL = 8;
const TEXTURE_FORMAT: GPUTextureFormat = 'rgba8unorm';

export class FlakesTextureGenerator {
  private _texture: GPUTexture | null = null;
  constructor(private device: GPUDevice, private size: number) {}

  get texture() {
    return this._texture;
  }

  generate() {
    const device = this.device;
    const shaderModule = device.createShaderModule({
      code: flakesTexture,
    });
    const renderPipeline = device.createRenderPipeline({
      layout: 'auto',
      vertex: {
        entryPoint: 'v_main',
        module: shaderModule,
        buffers: [
          {
            arrayStride: 24,
            stepMode: 'instance',
            attributes: [
              {
                // radius
                shaderLocation: 0,
                offset: 0,
                format: 'float32',
              },
              {
                // position
                shaderLocation: 1,
                offset: 4,
                format: 'float32x2',
              },
              {
                // color
                shaderLocation: 2,
                offset: 12,
                format: 'float32x3',
              },
            ],
          },
        ],
      },
      fragment: {
        entryPoint: 'f_main',
        module: shaderModule,
        targets: [{format: TEXTURE_FORMAT}],
      },
    });
    const instanceCount = 4000;

    const vertexBuffer = device.createBuffer({
      size: instanceCount * 24,
      usage: GPUBufferUsage.VERTEX | GPUBufferUsage.COPY_DST,
      mappedAtCreation: true,
    });
    new Float32Array(vertexBuffer.getMappedRange()).set(
      Array.from({length: instanceCount}).flatMap(() => {
        const r = 0.0118;

        let nx = Math.random() * 2 - 1;
        let ny = Math.random() * 2 - 1;
        let nz = 1.5;

        const l = Math.sqrt(nx * nx + ny * ny + nz * nz);

        nx /= l;
        ny /= l;
        nz /= l;
        return [
          r + Math.random() * r, // radius
          Math.random() * 2 - 1, // x
          Math.random() * 2 - 1, // y
          0.5 + nx * 0.5, // normal x
          0.5 + ny * 0.5, // normal y
          nz, // normal z
        ];
      }),
    );
    vertexBuffer.unmap();

    const commandEncoder = device.createCommandEncoder();

    const texture = device.createTexture({
      size: [this.size, this.size],
      format: TEXTURE_FORMAT,
      usage:
        GPUTextureUsage.RENDER_ATTACHMENT |
        GPUTextureUsage.TEXTURE_BINDING |
        GPUTextureUsage.COPY_SRC,
    });

    const renderPassDescriptor: GPURenderPassDescriptor = {
      colorAttachments: [
        {
          view: texture.createView(),
          loadOp: 'clear',
          clearValue: [0.5, 0.5, 1, 1],
          storeOp: 'store',
        },
      ],
    };

    const renderPassEncoder =
      commandEncoder.beginRenderPass(renderPassDescriptor);
    renderPassEncoder.setPipeline(renderPipeline);
    renderPassEncoder.setVertexBuffer(0, vertexBuffer);
    renderPassEncoder.draw(CIRCLE_POINTS * 3, instanceCount);
    renderPassEncoder.end();

    device.queue.submit([commandEncoder.finish()]);

    this._texture = texture;
  }

  async copyToImageData() {
    const device = this.device;
    const texture = this.texture;
    if (!texture) {
      throw new Error('Generate the texture first');
    }
    const bufferSize = this.size * this.size * 8;
    const buffer = device.createBuffer({
      size: bufferSize,
      usage: GPUBufferUsage.MAP_READ | GPUBufferUsage.COPY_DST,
    });

    const commandEncoder = device.createCommandEncoder();
    commandEncoder.copyTextureToBuffer(
      {
        texture,
      },
      {
        buffer,
        bytesPerRow: BYTES_PER_PIXEL * texture.width,
      },
      texture,
    );

    device.queue.submit([commandEncoder.finish()]);

    return buffer.mapAsync(GPUMapMode.READ).then(() => {
      const data = new Uint8ClampedArray(bufferSize);
      data.set(new Uint8ClampedArray(buffer.getMappedRange())); // make copy
      buffer.unmap();
      return {
        data,
        width: texture.width,
        height: texture.height,
      };
    });
  }
}
