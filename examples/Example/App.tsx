import {Examples} from './src/Components/Examples';
import {NavigationContainer} from '@react-navigation/native';
import {createNativeStackNavigator} from '@react-navigation/native-stack';
import type {Routes} from './src/types/navigationTypes';
import {ExampleScreen} from './src/Components/ExampleScreen';
import {Appearance} from 'react-native';
import {Root} from './src/Components/Root';

Appearance.setColorScheme('light');
const Stack = createNativeStackNavigator<Routes>();

export default function App() {
  return (
    <NavigationContainer>
      <Stack.Navigator
        screenOptions={{
          headerBackTitleVisible: false,
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
            title: route.params.name,
          })}
        />
      </Stack.Navigator>
    </NavigationContainer>
  );
}
