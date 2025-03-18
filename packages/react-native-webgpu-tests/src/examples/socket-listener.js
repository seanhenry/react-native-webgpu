import net from 'net';

const port = 8888;
const host = '127.0.0.1';

export class SocketListener {
  _messages = [];
  _server = null;
  async startListening() {
    return new Promise((resolve, reject) => {
      const server = net.createServer(socket => {
        socket.on('data', data => {
          this._messages.push(data.toString('utf8'));
        });
      });

      server.on('error', error => {
        reject(error);
      });
      server.on('listening', () => {
        resolve();
      });
      server.listen(port, host);
      this._server = server;
    });
  }

  close() {
    this._server?.close();
    this._server = null;
  }

  async waitForExample(example, id) {
    let timedOut = false;
    setTimeout(() => {
      timedOut = true;
    }, 15_000);
    while (true) {
      if (this.findExample(example, id)) {
        break;
      } else if (timedOut) {
        throw new Error(`Example ${example} did not finish`);
      }
      await new Promise(res => setTimeout(res, 1000));
    }
  }

  findExample(example, id) {
    for (let i = this._messages.length - 1; i >= 0; i--) {
      if (this._messages[i] === `${example}-${id}`) {
        return true;
      }
    }
    return false;
  }

  get lastMessage() {
    return this._messages[this._messages.length - 1];
  }
}
