import fs from 'node:fs';
import path from 'node:path';
import {LOGS_DIR} from './config.js';

/*
  Provides a prefixed print function for printing to console and fd/stdio to stream stdio logs to file, specified by fileName.
 */
export async function withLog(fileName, prefix, task) {
  const fd = fs.openSync(path.join(LOGS_DIR, fileName), 'w');
  try {
    const print = (...message) => console.log(prefix, ...message);
    await task({print, fd, stdio: ['inherit', fd, fd]});
  } finally {
    fs.close(fd);
  }
}
