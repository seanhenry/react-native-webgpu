let ids: Record<string, number> = {};

export const __resetIds = () => {
  ids = {};
};

export const getId = (key: string) => {
  ids[key] ??= 0;
  const id = ids[key];
  ids[key] += 1;
  return id;
};
