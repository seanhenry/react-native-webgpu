import {PropsWithChildren, useState} from 'react';
import {Pressable, View} from 'react-native';
import {globalStyles} from '../../globalStyles';
import {controlsStyles} from './controlsStyles';
import {ControlBackground} from './ControlBackground';
import {ControlLabel} from './ControlLabel';
import {ControlInput} from './ControlInput';

type ControlFolderProps = PropsWithChildren<{
  title: string;
  initialExpanded?: boolean;
  disabled: boolean;
}>;

export const ControlFolder = ({
  title,
  children,
  disabled,
  initialExpanded = false,
}: ControlFolderProps) => {
  const [expanded, setExpanded] = useState(initialExpanded);
  const caret = expanded ? 'v  ' : '>  ';
  return (
    <>
      <Pressable onPress={() => setExpanded(!expanded)} disabled={disabled}>
        {({pressed}) => (
          <ControlBackground dark>
            <View style={pressed && globalStyles.pressed}>
              <ControlLabel text={caret + title} />
            </View>
            <ControlInput />
          </ControlBackground>
        )}
      </Pressable>
      <View style={controlsStyles.controlsBackgroundColor}>
        {expanded ? children : null}
      </View>
    </>
  );
};
