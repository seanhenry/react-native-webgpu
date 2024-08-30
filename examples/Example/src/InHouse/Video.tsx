import {WebGpuView, WebGpuViewProps} from 'react-native-webgpu';
import {globalStyles} from '../Components/globalStyles';
import videoQuad from '../utils/shaders/videoQuad.wgsl';
import yuv8420torgba8 from '../utils/shaders/yuv8420torgba8.wgsl';
import {StyleSheet, View} from 'react-native';
import {useControls} from '../Components/controls/react/useControls';

const kFlipYFlag = 1;
const kIsInterleavedFlag = 2;

export const Video = () => {
  const {gui, Controls} = useControls();
  const onCreateSurface: WebGpuViewProps['onCreateSurface'] = async ({
    requestAnimationFrame,
    navigator,
    context,
  }) => {
    const adapter = await navigator.gpu.requestAdapter();
    const device = await adapter!.requestDevice();
    const preferredFormat = navigator.gpu.getPreferredCanvasFormat();
    const videoPlayer = reactNativeWebGPU.experimental.makeVideoPlayer(
      'https://test-videos.co.uk/vids/bigbuckbunny/mp4/h264/1080/Big_Buck_Bunny_1080_10s_2MB.mp4',
    );
    context.configure({
      device,
      format: preferredFormat,
    });
    const computeModule = device.createShaderModule({
      code: yuv8420torgba8,
    });
    const renderModule = device.createShaderModule({
      code: videoQuad,
    });
    const computePipeline = device.createComputePipeline({
      label: 'YUV8 420 To RGBA8',
      layout: 'auto',
      compute: {
        entryPoint: 'main',
        module: computeModule,
      },
    });
    const renderPipeline = device.createRenderPipeline({
      layout: 'auto',
      vertex: {
        entryPoint: 'v_main',
        module: renderModule,
      },
      fragment: {
        entryPoint: 'f_main',
        module: renderModule,
        targets: [{format: preferredFormat}],
      },
    });
    const sampler = device.createSampler({
      magFilter: 'nearest',
      minFilter: 'nearest',
    });
    const uniformBuffer = device.createBuffer({
      size: 4,
      usage: GPUBufferUsage.UNIFORM | GPUBufferUsage.COPY_DST,
    });
    const computeBindGroup1 = device.createBindGroup({
      layout: computePipeline.getBindGroupLayout(1),
      entries: [{binding: 0, resource: {buffer: uniformBuffer}}],
    });
    let rgba8Texture: GPUTexture | null = null;
    let computeBindGroup0: GPUBindGroup | null = null;
    let bindGroup: GPUBindGroup | null = null;
    let planeTextures: GPUTexture[] = [];

    const config = {
      flipY: true,
      isInterleaved: 'detected',
      restart: () => {
        videoPlayer.seek(0);
        videoPlayer.play();
      },
    };
    gui.add(config, 'flipY');
    gui.add(config, 'isInterleaved', ['detected', 'true', 'false']);
    gui.add(config, 'restart');
    gui.draw();

    const frame = () => {
      const framebuffer = context.getCurrentTexture();
      const pixelBuffer = videoPlayer.nextPixelBuffer();
      if (!framebuffer || !pixelBuffer) {
        requestAnimationFrame(frame);
        return;
      }
      if (planeTextures.length === 0) {
        planeTextures = new Array(pixelBuffer.planeCount).fill(0).map((_, i) =>
          device.createTexture({
            format: 'r8unorm',
            size: [
              pixelBuffer.getWidthOfPlane(i),
              pixelBuffer.getHeightOfPlane(i),
            ],
            usage: GPUTextureUsage.TEXTURE_BINDING | GPUTextureUsage.COPY_DST,
          }),
        );
      }
      if (!rgba8Texture) {
        rgba8Texture = device.createTexture({
          format: 'rgba8unorm',
          usage:
            GPUTextureUsage.TEXTURE_BINDING | GPUTextureUsage.STORAGE_BINDING,
          size: pixelBuffer,
        });
      }
      if (!computeBindGroup0) {
        computeBindGroup0 = device.createBindGroup({
          layout: computePipeline.getBindGroupLayout(0),
          entries: [
            {binding: 0, resource: rgba8Texture.createView()},
            {binding: 1, resource: planeTextures[0].createView()},
            {binding: 2, resource: planeTextures[1].createView()},
            {
              binding: 3,
              resource:
                planeTextures[
                  Math.min(pixelBuffer.planeCount - 1, 2)
                ].createView(),
            },
          ],
        });
      }
      if (!bindGroup) {
        bindGroup = device.createBindGroup({
          layout: renderPipeline.getBindGroupLayout(0),
          entries: [
            {binding: 0, resource: sampler},
            {binding: 1, resource: rgba8Texture.createView()},
          ],
        });
      }
      planeTextures.forEach((texture, i) => {
        device.queue.writeTexture(
          {texture},
          pixelBuffer.getArrayBufferOfPlane(i),
          {bytesPerRow: pixelBuffer.getBytesPerRowOfPlane(i)},
          texture,
        );
      });
      let isInterleaved = pixelBuffer.isInterleaved;
      if (config.isInterleaved !== 'detected') {
        isInterleaved = config.isInterleaved === 'true';
      }

      device.queue.writeBuffer(
        uniformBuffer,
        0,
        new Uint32Array([
          (config.flipY ? kFlipYFlag : 0) |
            (isInterleaved ? kIsInterleavedFlag : 0),
        ]),
      );
      const commandEncoder = device.createCommandEncoder();
      const computePass = commandEncoder.beginComputePass();
      computePass.setPipeline(computePipeline);
      computePass.setBindGroup(0, computeBindGroup0);
      computePass.setBindGroup(1, computeBindGroup1);
      computePass.dispatchWorkgroups(pixelBuffer.width, pixelBuffer.height);
      computePass.end();
      const renderPass = commandEncoder.beginRenderPass({
        colorAttachments: [
          {
            view: framebuffer.createView(),
            storeOp: 'store',
            loadOp: 'clear',
            clearValue: [0, 0, 0, 1],
          },
        ],
      });
      renderPass.setPipeline(renderPipeline);
      renderPass.setBindGroup(0, bindGroup);
      renderPass.draw(6);
      renderPass.end();
      device.queue.submit([commandEncoder.finish()]);
      context.presentSurface();
      pixelBuffer.release();

      requestAnimationFrame(frame);
    };
    requestAnimationFrame(frame);
    return () => {
      videoPlayer.release();
    };
  };
  return (
    <>
      <View style={styles.container}>
        <WebGpuView
          onCreateSurface={onCreateSurface}
          style={globalStyles.fill}
        />
      </View>
      <Controls />
    </>
  );
};

const styles = StyleSheet.create({
  container: {
    width: '100%',
    aspectRatio: 16 / 9,
  },
});
