import {Square} from '../Components/Square';
import {WebGpuView, WebGpuViewProps} from 'react-native-webgpu';
import {globalStyles} from '../Components/globalStyles';
import {HudContainer} from '../Components/stats/HudContainer';
import {useHudText} from '../Components/stats/useHudText';
import {useControls} from '../Components/controls/react/useControls';
import {Controller} from '../Components/controls/dat.gui/Controller';

export const ExceedDeviceLimits = () => {
  const {gui, Controls} = useControls();
  const {setText, HudText} = useHudText();
  const onCreateSurface: WebGpuViewProps['onCreateSurface'] = async ({
    navigator,
  }) => {
    const adapter = await navigator.gpu.requestAdapter();

    const onChange = async (key: string, value: number) => {
      try {
        await adapter!.requestDevice({
          requiredLimits: {[key]: value},
        });
        setText("The device didn't report an error");
      } catch (error) {
        setText(`The device reported the error: ${error}`);
      }
    };
    const controllers: Controller[] = [];
    const resetLimits = () => {
      const limits = adapter!.limits;
      controllers.push(
        ...Object.keys(limits).map(key =>
          gui
            .add(limits, key as keyof GPUSupportedLimits)
            .onChange(newValue => onChange(key, newValue)),
        ),
      );
    };
    const controls = {
      reset: () => {
        controllers.forEach(controller => {
          controller.remove();
        });
        resetLimits();
        gui.draw();
      },
    };
    gui.add(controls, 'reset');
    resetLimits();

    gui.draw();
  };
  return (
    <>
      <Square>
        <WebGpuView
          onCreateSurface={onCreateSurface}
          style={globalStyles.fill}
        />
      </Square>
      <HudContainer>
        <HudText />
      </HudContainer>
      <Controls />
    </>
  );
};
