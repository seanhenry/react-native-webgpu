import {List} from './List';
import {type NavigationProp, useNavigation} from '@react-navigation/native';
import type {Routes} from '../types/navigationTypes';
import {Examples} from './ExampleScreen';
import {useSafeAreaInsets} from 'react-native-safe-area-context';

export function Root() {
  const navigation = useNavigation<NavigationProp<Routes, 'example'>>();
  const insets = useSafeAreaInsets();

  return (
    <List<Examples>
      insets={insets}
      onPressItem={item => navigation.navigate('examples', {name: item.title})}
      sections={[
        {
          title: 'Examples',
          data: [{title: 'WebGPUSamples'}, {title: 'Three'}],
        },
      ]}
    />
  );
}
