import {Pressable, SectionList, StyleSheet, Text, View} from 'react-native';
import type {
  SectionListData,
  SectionListRenderItem,
} from 'react-native/Libraries/Lists/SectionList';
import {EdgeInsets} from 'react-native-safe-area-context';

type HeaderItem = {
  title: string;
};

type ListItem<T extends string> = {
  title: T;
};

export type ListProps<T extends string> = {
  sections: ReadonlyArray<SectionListData<ListItem<T>, HeaderItem>>;
  onPressItem: (item: ListItem<T>) => void;
  insets?: EdgeInsets;
};

export function List<T extends string>({
  sections,
  onPressItem,
  insets = {top: 0, bottom: 0, left: 0, right: 0},
}: ListProps<T>) {
  const renderSectionHeader = ({
    section,
  }: {
    section: SectionListData<ListItem<T>, HeaderItem>;
  }) => (
    <View style={styles.headerItem}>
      <Text style={styles.headerText}>{section.title}</Text>
    </View>
  );
  const renderItem: SectionListRenderItem<ListItem<T>> = ({item, index}) => (
    <Pressable onPress={() => onPressItem(item)}>
      {({pressed}) => (
        <>
          <View style={styles.separator} />
          <View
            style={[
              styles.listItem,
              index % 2 === 0 ? styles.evenBackground : styles.oddBackground,
            ]}>
            <Text style={[styles.text, pressed && styles.pressed]}>
              {item.title}
            </Text>
          </View>
        </>
      )}
    </Pressable>
  );
  return (
    <>
      <SectionList<ListItem<T>, HeaderItem>
        sections={sections}
        automaticallyAdjustContentInsets={false}
        automaticallyAdjustsScrollIndicatorInsets={false}
        contentInset={{top: insets.top, bottom: insets.bottom}}
        contentOffset={{x: 0, y: -insets.top}}
        style={styles.container}
        renderSectionHeader={renderSectionHeader}
        renderItem={renderItem}
      />
      <View style={[styles.headerBackground, {height: insets.top}]} />
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
