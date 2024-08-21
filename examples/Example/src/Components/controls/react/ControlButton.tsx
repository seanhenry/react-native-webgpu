import {Button, ButtonProps} from 'react-native';
import {controlForegroundColor} from './controlsStyles';

type ControlButtonProps = Pick<ButtonProps, 'title' | 'disabled' | 'onPress'>;

export const ControlButton = ({
  title,
  onPress,
  disabled,
}: ControlButtonProps) => {
  return (
    <Button
      title={title}
      onPress={onPress}
      disabled={disabled}
      color={controlForegroundColor}
    />
  );
};
