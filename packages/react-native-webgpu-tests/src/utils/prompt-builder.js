import select from '@inquirer/select';
import {REACT_NATIVE_VERSIONS} from './config.js';

export class PromptBuilder {
  prompts = [];

  skipPack() {
    this.prompts.push([
      'skipPack',
      () =>
        select({
          message: 'Skip pack?',
          choices: [
            {
              name: 'No',
              value: false,
            },
            {
              name: 'Yes',
              value: true,
            },
          ],
        }),
    ]);
    return this;
  }

  platforms() {
    this.prompts.push([
      'platforms',
      () =>
        select({
          message: 'Platforms',
          choices: [
            {
              name: 'Android and iOS',
              value: ['android', 'ios'],
            },
            {
              name: 'Android',
              value: ['android'],
            },
            {
              name: 'iOS',
              value: ['ios'],
            },
          ],
        }),
    ]);
    return this;
  }

  reactNativeVersions() {
    this.prompts.push([
      'reactNativeVersions',
      () =>
        select({
          message: 'RN versions',
          choices: [
            {
              name: 'All',
              value: [...REACT_NATIVE_VERSIONS],
              description: `All versions: ${REACT_NATIVE_VERSIONS.join(', ')}`,
            },
            ...REACT_NATIVE_VERSIONS.map(version => ({
              name: version,
              value: [version],
            })),
          ],
        }),
    ]);
    return this;
  }

  configurations() {
    this.prompts.push([
      'configurations',
      () =>
        select({
          message: 'Configurations',
          choices: [
            {
              name: 'Release',
              value: ['Release'],
            },
            {
              name: 'Debug',
              value: ['Debug'],
            },
            {
              name: 'Release and Debug',
              value: ['Release', 'Debug'],
            },
          ],
        }),
    ]);
    return this;
  }

  architectures() {
    this.prompts.push([
      'architectures',
      () =>
        select({
          message: 'RN architectures',
          choices: [
            {
              name: 'New and old architectures',
              value: ['newarch', 'oldarch'],
            },
            {
              name: 'New',
              value: ['newarch'],
            },
            {
              name: 'Old',
              value: ['oldarch'],
            },
          ],
        }),
    ]);
    return this;
  }

  async prompt() {
    const result = {};
    for (const [key, prompt] of this.prompts) {
      result[key] = await prompt();
    }
    return result;
  }
}
