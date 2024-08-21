import {Text} from 'react-native';
import {hudStyles} from './hudStyles';

type HudLabelProps = {
  text: string;
};

export const HudLabel = ({text}: HudLabelProps) => (
  <Text style={hudStyles.text}>{text}</Text>
);
