import {Examples} from './Components/Examples';
import {NavigationContainer} from '@react-navigation/native';
import {createNativeStackNavigator} from '@react-navigation/native-stack';
import type {Routes} from './types/navigationTypes';
import {ExampleScreen} from './Components/ExampleScreen';
import {Appearance} from 'react-native';
import {Root} from './Components/Root';
import {GestureHandlerRootView} from 'react-native-gesture-handler';
import Animated from 'react-native-reanimated';
import {launchArguments} from './utils/launchArguments';

Appearance.setColorScheme('light');
const Stack = createNativeStackNavigator<Routes>();
Animated.addWhitelistedNativeProps({text: true});

export default function App() {
  return (
    <GestureHandlerRootView>
      <NavigationContainer>
        <Stack.Navigator
          initialRouteName={launchArguments.example ? 'example' : 'root'}
          screenOptions={{
            headerBackButtonDisplayMode: 'minimal',
            headerTintColor: 'black',
          }}>
          <Stack.Screen
            name="root"
            component={Root}
            options={{headerShown: false}}
          />
          <Stack.Screen name="examples" component={Examples} />
          <Stack.Screen
            name="example"
            component={ExampleScreen}
            options={({route}) => ({
              title: route.params?.name ?? launchArguments.example ?? '',
            })}
          />
        </Stack.Navigator>
      </NavigationContainer>
    </GestureHandlerRootView>
  );
}
