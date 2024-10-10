'use strict';
import type { TurboModule } from 'react-native';
import { TurboModuleRegistry } from 'react-native';

interface Spec extends TurboModule {
  readonly getConstants: () => { ENABLE_THREADS: boolean };

  installWithThreadId(threadId: string): boolean;
}

export default TurboModuleRegistry.get<Spec>('WGPUJsi');
