import {Square} from '../Components/Square';
import {WebGpuView, WebGpuViewProps} from 'react-native-webgpu';
import {globalStyles} from '../Components/globalStyles';
import {HudContainer} from '../Components/stats/HudContainer';
import {useHudText} from '../Components/stats/useHudText';
import {useControls} from '../Components/controls/react/useControls';

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
    const resetGui = () => {
      const limits = adapter!.limits;
      gui.removeItems();
      gui.add({resetGui}, 'resetGui').name('reset');
      Object.keys(limits)
        .filter(key => key !== '__brand')
        .forEach(key =>
          gui
            .add(limits, key as keyof GPUSupportedLimits)
            .onChange(newValue => onChange(key, newValue)),
        );
      gui.draw();
    };
    resetGui();
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
