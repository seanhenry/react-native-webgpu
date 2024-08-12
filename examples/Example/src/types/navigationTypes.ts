import type {Examples, Example} from '../Components/ExampleScreen';

export type Routes = {
  root: undefined;
  examples: {
    name: Examples;
  };
  example: {
    name: Example;
  };
};
