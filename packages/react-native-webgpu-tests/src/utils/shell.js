import {promisify} from 'node:util';
import childProcess from 'node:child_process';
import fs from 'node:fs/promises';

export const exec = promisify(childProcess.exec);
export const spawn = (cmd, argsOrOpts, maybeOpts) =>
  new Promise((resolve, reject) => {
    const error = new Error();
    const args = Array.isArray(argsOrOpts) ? argsOrOpts : [];
    const opts = !Array.isArray(argsOrOpts) ? argsOrOpts : maybeOpts;
    const process = childProcess.spawn(
      cmd,
      args,
      Object.assign(opts ?? {}, {shell: true}),
    );
    process.on('error', err => {
      error.message =
        'The child process ended with an error. See underlyingError.';
      error.underlyingError = err;
      reject(error);
    });
    process.on('close', code => {
      if (code === 0) {
        resolve();
      } else {
        error.message = `Command failed with code ${code}`;
        reject(error);
      }
    });
  });

export async function replaceFileContents(file, searchValue, replaceValue) {
  const contents = await fs.readFile(file, {encoding: 'utf8'});
  await fs.writeFile(file, contents.replace(searchValue, replaceValue), {
    encoding: 'utf8',
  });
}

export async function rm(opts, ...files) {
  await Promise.all(
    files.map(file =>
      fs.rm(file, {recursive: opts.includes('r'), force: opts.includes('f')}),
    ),
  );
}

export async function mkdir(opts, ...files) {
  for (const file of files) {
    await fs.mkdir(file, {recursive: opts.includes('p')});
  }
}

export async function mv(from, to) {
  await fs.rename(from, to);
}
