import {installWithThreadId} from 'react-native-webgpu';
import {installWithThreadId as installExperimentalWithThreadId} from 'react-native-webgpu-experimental';
import triangleVertWGSL from './src/WebGPUSamples/shaders/triangle.vert.wgsl';
import redFragWGSL from './src/WebGPUSamples/shaders/red.frag.wgsl';

global.reactNativeWebGPUThreadsInstance.onAttachSurface = ({uuid}) => {
  const run = async ({context, navigator, requestAnimationFrame}) => {
    const adapter = await navigator.gpu.requestAdapter();
    const device = await adapter.requestDevice();

    const presentationFormat = navigator.gpu.getPreferredCanvasFormat();

    context.configure({
      device,
      format: presentationFormat,
      alphaMode: 'auto',
    });

    const pipeline = device.createRenderPipeline({
      layout: 'auto',
      vertex: {
        entryPoint: 'main',
        module: device.createShaderModule({
          code: triangleVertWGSL,
        }),
      },
      fragment: {
        entryPoint: 'main',
        module: device.createShaderModule({
          code: redFragWGSL,
        }),
        targets: [
          {
            format: presentationFormat,
          },
        ],
      },
      primitive: {
        topology: 'triangle-list',
      },
    });

    function frame() {
      const framebuffer = context.getCurrentTexture();
      if (!framebuffer) {
        requestAnimationFrame(frame);
        return;
      }

      const commandEncoder = device.createCommandEncoder();
      const textureView = framebuffer.createView();

      const renderPassDescriptor = {
        colorAttachments: [
          {
            view: textureView,
            clearValue: [0, 0, 0, 1],
            loadOp: 'clear',
            storeOp: 'store',
          },
        ],
      };

      const passEncoder = commandEncoder.beginRenderPass(renderPassDescriptor);
      passEncoder.setPipeline(pipeline);
      passEncoder.draw(3);
      passEncoder.end();

      device.queue.submit([commandEncoder.finish()]);
      context.presentSurface();
      requestAnimationFrame(frame);
    }

    requestAnimationFrame(frame);
  };
  const webGPU = global.reactNativeWebGPU.getSurfaceBackedWebGPU(uuid);
  run(webGPU);
};

const threadId = global.reactNativeWebGPUThreadsInstance.getContext().threadId;
installWithThreadId(threadId);
installExperimentalWithThreadId(threadId);
