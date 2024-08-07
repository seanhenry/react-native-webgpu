import * as React from 'react';
import { Examples } from './Components/Examples';
import { NavigationContainer } from '@react-navigation/native';
import { createNativeStackNavigator } from '@react-navigation/native-stack';
import type { Routes } from './types/navigationTypes';
import { ExampleScreen } from './Components/ExampleScreen';

const Stack = createNativeStackNavigator<Routes>();

export default function App() {
  return (
    <NavigationContainer>
      <Stack.Navigator screenOptions={{
        headerBackTitleVisible: false,
        headerTintColor: 'black',
      }}>
        <Stack.Screen name="root" component={Examples} options={{ headerShown: false }} />
        <Stack.Screen name="example" component={ExampleScreen} options={({route}) => ({
          title: route.params.name,
        })} />
      </Stack.Navigator>
    </NavigationContainer>
  );
}
