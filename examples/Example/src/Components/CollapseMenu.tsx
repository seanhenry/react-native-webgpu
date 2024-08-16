import {Button, Pressable, StyleSheet, Text, View} from 'react-native';
import {useState} from 'react';

type CollapseMenuProps = {
  title: string;
  actions: {
    title: string;
    onPress: () => void;
  }[];
};

export const CollapseMenu = ({title, actions}: CollapseMenuProps) => {
  const [expanded, setExpanded] = useState(false);
  const caret = expanded ? 'v  ' : '>  ';
  return (
    <View style={styles.container}>
      <Pressable onPress={() => setExpanded(!expanded)}>
        {({pressed}) => (
          <Text style={pressed && styles.pressed}>{caret + title} </Text>
        )}
      </Pressable>
      {expanded
        ? actions.map(({title, onPress}) => (
            <Button
              key={title}
              title={title}
              onPress={() => {
                setExpanded(false);
                onPress();
              }}
            />
          ))
        : null}
    </View>
  );
};

const styles = StyleSheet.create({
  controls: {
    position: 'absolute',
    top: 0,
    left: 0,
    right: 0,
  },
  container: {
    padding: 8,
    backgroundColor: '#EFEFEF',
  },
  pressed: {
    opacity: 0.6,
  },
});
