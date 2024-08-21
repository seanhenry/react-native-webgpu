import {__resetIds, getId} from './incrementingId';

describe('getId', () => {
  beforeEach(() => {
    __resetIds();
  });

  it('gets id', () => {
    expect(getId('test')).toBe(0);
    expect(getId('test')).toBe(1);
    __resetIds();
    expect(getId('test')).toBe(0);
  });
});
