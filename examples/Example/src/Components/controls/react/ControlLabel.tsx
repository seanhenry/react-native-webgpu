import {Text} from 'react-native';
import {controlsStyles} from './controlsStyles';

type ControlLabelProps = {
  text: string;
};

export const ControlLabel = ({text}: ControlLabelProps) => {
  return <Text style={controlsStyles.text}>{text}</Text>;
};
