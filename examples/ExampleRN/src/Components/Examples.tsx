import type { ComponentType } from 'react';
import * as React from 'react';

import { Pressable, SectionList, StyleSheet, Text, View } from 'react-native';
import { HelloTriangle } from '../WebGPUSamples/BasicGraphics/HelloTriangle/HelloTriangle';
import { HelloTriangleMSAA } from '../WebGPUSamples/BasicGraphics/HelloTriangleMSAA/HelloTriangleMSAA';
import { RotatingCube } from '../WebGPUSamples/BasicGraphics/RotatingCube/RotatingCube';
import { TwoCubes } from '../WebGPUSamples/BasicGraphics/TwoCubes/TwoCubes';
import { TexturedCube } from '../WebGPUSamples/BasicGraphics/TexturedCube/TexturedCube';
import { InstancedCube } from '../WebGPUSamples/BasicGraphics/InstancedCube/InstancedCube';
import { FractalCube } from '../WebGPUSamples/BasicGraphics/FractalCube/FractalCube';
import { CubeMap } from '../WebGPUSamples/BasicGraphics/CubeMap/CubeMap';
import { ComputeBoids } from '../WebGPUSamples/GPGPU/ComputeBoids/ComputeBoids';
import { GameOfLife } from '../WebGPUSamples/GPGPU/GameOfLife/GameOfLife';
import { BitonicSort } from '../WebGPUSamples/GPGPU/BitonicSort/BitonicSort';
import type { SectionListData, SectionListRenderItem } from 'react-native/Libraries/Lists/SectionList';
import { useSafeAreaInsets } from 'react-native-safe-area-context';
import type { Example } from './ExampleScreen';
import { type NavigationProp, useNavigation } from '@react-navigation/native';
import type { Routes } from '../types/navigationTypes';

type HeaderItem = {
  title: string;
}

type ListItem = {
  title: Example;
  Component: ComponentType;
}

export function Examples() {
  const insets = useSafeAreaInsets();
  const navigation = useNavigation<NavigationProp<Routes, 'example'>>()
  const renderSectionHeader = ({ section }: { section: SectionListData<ListItem, HeaderItem> }) => (
    <View style={styles.headerItem}>
      <Text style={styles.headerText}>{section.title}</Text>
    </View>
  );
  const renderItem: SectionListRenderItem<ListItem> = ({ item, index }) => (
    <Pressable onPress={() => navigation.navigate('example', {name: item.title})}>
      {({pressed}) => (
        <>
          <View style={styles.separator} />
          <View style={[styles.listItem, index % 2 === 0 ? styles.evenBackground : styles.oddBackground]}>
            <Text style={[styles.text, pressed && styles.pressed]}>{item.title}</Text>
          </View>
        </>
      )}
    </Pressable>
  );
  return (
    <>
      <SectionList<ListItem, HeaderItem>
        sections={[
          {
            title: 'Basic Graphics',
            data: [
              { title: 'HelloTriangle', Component: HelloTriangle },
              { title: 'HelloTriangleMSAA', Component: HelloTriangleMSAA },
              { title: 'RotatingCube', Component: RotatingCube },
              { title: 'TwoCubes', Component: TwoCubes },
              { title: 'TexturedCube', Component: TexturedCube },
              { title: 'InstancedCube', Component: InstancedCube },
              { title: 'FractalCube', Component: FractalCube },
              { title: 'CubeMap', Component: CubeMap },
            ],
          },
          {
            title: 'GPGPU',
            data: [
              { title: 'ComputeBoids', Component: ComputeBoids },
              { title: 'GameOfLife', Component: GameOfLife },
              { title: 'BitonicSort', Component: BitonicSort },
            ],
          },
        ]}
        automaticallyAdjustContentInsets={false}
        automaticallyAdjustsScrollIndicatorInsets={false}
        contentInset={{ top: insets.top, bottom: insets.bottom }}
        contentOffset={{x: 0, y: -insets.top}}
        style={styles.container}
        renderSectionHeader={renderSectionHeader}
        renderItem={renderItem} />
      <View style={[styles.headerBackground, { height: insets.top }]} />
    </>
  );
}

const styles = StyleSheet.create({
  container: {
    flex: 1,
  },
  headerBackground: {
    position: 'absolute',
    left: 0,
    right: 0,
    top: 0,
    backgroundColor: '#FFFFFFCC',
  },
  listItem: {
    paddingHorizontal: 8,
    paddingVertical: 20,
  },
  pressed: {
    opacity: 0.6,
  },
  text: {
    fontSize: 20,
  },
  headerItem: {
    paddingHorizontal: 8,
    paddingVertical: 8,
    backgroundColor: '#FFFFFFCC',
  },
  headerText: {
    fontSize: 20,
    fontWeight: 'bold',
  },
  separator: {
    paddingLeft: 8,
    backgroundColor: '#EFEFEF',
    height: 1,
  },
  evenBackground: {
    backgroundColor: '#EFEFEF',
  },
  oddBackground: {
    backgroundColor: '#FEFEFE',
  },
});
