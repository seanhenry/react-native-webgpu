import {ThreadWebGpuView} from 'react-native-webgpu';
import {globalStyles} from '../Components/globalStyles';
import {Square} from '../Components/Square';

export const Thread = () => {
  return (
    <Square>
      <ThreadWebGpuView threadId="myThread-0" style={globalStyles.fill} />
    </Square>
  );
};
