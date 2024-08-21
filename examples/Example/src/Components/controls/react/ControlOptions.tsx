import {Button, View} from 'react-native';
import {useEffect, useState} from 'react';
import {Observable} from '../../../utils/observable';
import {ControlBackground} from './ControlBackground';
import {ControlLabel} from './ControlLabel';
import {ControlInput} from './ControlInput';
import {ControlFolder} from './ControlFolder';
import {controlForegroundColor, controlsStyles} from './controlsStyles';

type ControlOptionsProps = {
  title: string;
  disabled: boolean;
  initialValue: string;
  options: {
    title: string;
    onPress: () => void;
  }[];
  initialExpanded?: boolean;
  observable: Observable<string>;
};

export const ControlOptions = ({
  title,
  disabled,
  options,
  initialValue,
  initialExpanded = false,
  observable,
}: ControlOptionsProps) => {
  const [selected, setSelected] = useState(initialValue);
  useEffect(() => observable.add(setSelected), [observable]);
  return (
    <ControlBackground>
      <ControlLabel text={title} />
      <ControlInput>
        <View>
          <ControlFolder
            title={selected}
            initialExpanded={initialExpanded}
            disabled={disabled}>
            <View style={controlsStyles.controlsBackgroundColor}>
              {options.map(option => (
                <Button
                  key={option.title}
                  title={option.title}
                  disabled={selected === option.title}
                  color={controlForegroundColor}
                  onPress={() => {
                    setSelected(option.title);
                    option.onPress();
                  }}
                />
              ))}
            </View>
          </ControlFolder>
        </View>
      </ControlInput>
    </ControlBackground>
  );
};
