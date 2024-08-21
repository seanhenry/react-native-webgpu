import {forwardRef} from 'react';
import {TextInput, TextInputProps} from 'react-native';
import {hudStyles} from './hudStyles';

export const HudText = forwardRef<TextInput, TextInputProps>(
  ({style, ...props}, ref) => (
    <TextInput
      ref={ref}
      editable={false}
      multiline
      style={[hudStyles.text, style]}
      {...props}
    />
  ),
);
HudText.displayName = 'HudText';
