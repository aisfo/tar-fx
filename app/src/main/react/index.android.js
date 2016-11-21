import React, { Component } from 'react';
import {
  AppRegistry,
  StyleSheet,
  Text,
  View,
  NativeModules
} from 'react-native';

var ReactModule = NativeModules.ReactModule;

class TarFX extends Component {
  render() {
    ReactModule.test()
        .then(r => {
            console.log(r);
        });

    return (
      <View style={styles.container}>
        <Text style={styles.welcome}>
          TarFX
        </Text>
      </View>
    );
  }
}

const styles = StyleSheet.create({
  container: {
    flex: 1,
    justifyContent: 'center',
    alignItems: 'center',
    backgroundColor: '#F5FCFF',
  },
  welcome: {
    fontSize: 20,
    textAlign: 'center',
    margin: 10,
  }
});

AppRegistry.registerComponent('TarFX', () => TarFX);
