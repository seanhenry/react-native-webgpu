import {Square} from '../Components/Square';
import {WebGpuView, WebGpuViewProps} from 'react-native-webgpu';
import {globalStyles} from '../Components/globalStyles';
import {useControls} from '../Components/controls/react/useControls';
import {HudContainer} from '../Components/stats/HudContainer';
import {useHudText} from '../Components/stats/useHudText';

export const AdapterInfo = () => {
  const {gui, Controls} = useControls();
  const {setText, HudText} = useHudText();
  const onCreateSurface: WebGpuViewProps['onCreateSurface'] = async ({
    navigator,
  }) => {
    const adapter = await navigator.gpu.requestAdapter();

    const setInfo = (methodName: string, info: GPUAdapterInfo) => {
      setText(`${methodName}
Vendor: ${info.vendor}
Architecture: ${info.architecture}
Device: ${info.device}
Description: ${info.description}`);
    };
    const methods = {
      info: () => setInfo('adapter.info', adapter!.info),
      requestAdapterInfo: () =>
        adapter!.requestAdapterInfo().then(info => {
          setInfo('adapter.requestAdapterInfo()', info);
        }),
    };
    gui.add(methods, 'info');
    gui.add(methods, 'requestAdapterInfo');
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
