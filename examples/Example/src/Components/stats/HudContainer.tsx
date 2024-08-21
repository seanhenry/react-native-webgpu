import {PropsWithChildren} from 'react';
import {View} from 'react-native';
import {hudStyles} from './hudStyles';

type HudContainerProps = PropsWithChildren;

export const HudContainer = ({children}: HudContainerProps) => {
  return <View style={hudStyles.container}>{children}</View>;
};
