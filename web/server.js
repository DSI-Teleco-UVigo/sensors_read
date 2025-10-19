const http = require('http');
const fs = require('fs');
const path = require('path');
const { spawn } = require('child_process');

const PORT = process.env.PORT || 3000;
const EXECUTABLE_PATH = path.resolve(__dirname, '../build/sensors_read_test');
const SAMPLE_PATH = path.resolve(__dirname, 'output_sample.txt');
const BLOCK_PREFIX = '===== Sensor Readings';

const clients = new Set();
let buffer = '';
let childProcess = null;
let fallbackTimer = null;
let keepAliveTimer = null;

function stripAnsi(input) {
  return input.replace(/\x1b\[[0-9;]*[A-Za-z]/g, '');
}

function sendToClients(payload) {
  if (clients.size === 0) {
    return;
  }

  const data = `data: ${JSON.stringify(payload)}\n\n`;
  for (const res of clients) {
    res.write(data);
  }
}

function parseBlock(block) {
  const lines = block
    .split(/\r?\n/)
    .map((line) => line.trim())
    .filter((line) => line.length > 0);

  if (lines.length === 0) {
    return;
  }

  const header = lines[0];
  const headerMatch = header.match(/^===== Sensor Readings at\s+(\d+)/);
  const readingTimestamp = headerMatch ? Number(headerMatch[1]) : null;

  const sensors = {};

  for (let i = 1; i < lines.length; i += 1) {
    const line = lines[i];
    const separatorIndex = line.indexOf(':');
    if (separatorIndex === -1) {
      continue;
    }

    const label = line.slice(0, separatorIndex).trim();
    const remainder = line.slice(separatorIndex + 1).trim();
    const fields = remainder.split(/\s+/);
    const data = {};

    for (const field of fields) {
      const [key, value] = field.split('=');
      if (!key || value === undefined) {
        continue;
      }

      const numericValue = Number(value);
      data[key] = Number.isNaN(numericValue) ? value : numericValue;
    }

    sensors[label] = data;
  }

  const payload = {
    receivedAt: Date.now(),
    readingTimestamp,
    sensors,
  };

  const imu = sensors['IMU/MPU9250'];
  if (imu && typeof imu.ax === 'number' && typeof imu.ay === 'number' && typeof imu.az === 'number') {
    payload.accel = {
      ax: imu.ax,
      ay: imu.ay,
      az: imu.az,
    };
  }

  sendToClients(payload);
}

function consumeBuffer() {
  let searchStart = buffer.indexOf(BLOCK_PREFIX);
  while (searchStart !== -1) {
    const nextStart = buffer.indexOf(BLOCK_PREFIX, searchStart + BLOCK_PREFIX.length);
    if (nextStart === -1) {
      if (searchStart > 0) {
        buffer = buffer.slice(searchStart);
      }
      return;
    }

    const block = buffer.slice(searchStart, nextStart).trim();
    if (block.startsWith(BLOCK_PREFIX)) {
      parseBlock(block);
    }
    buffer = buffer.slice(nextStart);
    searchStart = buffer.indexOf(BLOCK_PREFIX);
  }

  if (buffer.length > 5000) {
    buffer = buffer.slice(-2000);
  }
}

function handleSensorData(chunk) {
  buffer += stripAnsi(chunk.toString('utf8'));
  consumeBuffer();
}

function startFallback() {
  if (fallbackTimer) {
    return;
  }

  let sampleBlock = null;
  try {
    sampleBlock = fs.readFileSync(SAMPLE_PATH, 'utf8');
  } catch (err) {
    console.error('Unable to read sample output:', err);
    return;
  }

  console.warn('Starting fallback mode using sample data.');
  fallbackTimer = setInterval(() => parseBlock(sampleBlock), 1000);
}

function startSensorProcess() {
  try {
    childProcess = spawn(EXECUTABLE_PATH, [], { stdio: ['ignore', 'pipe', 'pipe'] });
  } catch (error) {
    console.error('Failed to spawn sensor reader:', error);
    startFallback();
    return;
  }

  childProcess.stdout.on('data', handleSensorData);
  childProcess.stderr.on('data', (data) => {
    console.error('[sensors stderr]', data.toString());
  });
  childProcess.on('error', (error) => {
    console.error('Sensor process error:', error);
    startFallback();
  });
  childProcess.on('exit', (code, signal) => {
    console.warn(`Sensor process exited (code=${code}, signal=${signal}).`);
    childProcess = null;
    startFallback();
  });
}

function serveStaticFile(res, relativePath, contentType) {
  const absolutePath = path.resolve(__dirname, relativePath);
  const stream = fs.createReadStream(absolutePath);

  stream.on('open', () => {
    res.writeHead(200, { 'Content-Type': contentType });
    stream.pipe(res);
  });

  stream.on('error', (err) => {
    res.writeHead(500);
    res.end(`Failed to read ${relativePath}: ${err.message}`);
  });
}

const server = http.createServer((req, res) => {
  if (req.url === '/stream') {
    res.writeHead(200, {
      'Content-Type': 'text/event-stream',
      'Cache-Control': 'no-cache',
      Connection: 'keep-alive',
      'Access-Control-Allow-Origin': '*',
    });
    res.write('\n');
    clients.add(res);

    req.on('close', () => {
      clients.delete(res);
    });
    return;
  }

  if (req.url === '/' || req.url === '/index.html') {
    serveStaticFile(res, 'public/index.html', 'text/html; charset=utf-8');
    return;
  }

  if (req.url === '/client.js') {
    serveStaticFile(res, 'public/client.js', 'application/javascript; charset=utf-8');
    return;
  }

  if (req.url === '/styles.css') {
    serveStaticFile(res, 'public/styles.css', 'text/css; charset=utf-8');
    return;
  }

  res.writeHead(404);
  res.end('Not found');
});

const HOST = process.env.HOST || '0.0.0.0';

server.listen(PORT, HOST, () => {
  console.log(`Server listening on http://${HOST}:${PORT}`);
  startSensorProcess();
  if (!keepAliveTimer) {
    keepAliveTimer = setInterval(() => {
      if (clients.size === 0) {
        return;
      }
      for (const res of clients) {
        res.write(': keep-alive\n\n');
      }
    }, 15000);
  }
});

function shutdown() {
  console.log('Shutting down server...');
  if (childProcess) {
    childProcess.kill();
  }
  if (fallbackTimer) {
    clearInterval(fallbackTimer);
  }
  if (keepAliveTimer) {
    clearInterval(keepAliveTimer);
  }
  for (const res of clients) {
    res.end();
  }
  server.close(() => {
    process.exit(0);
  });
}

process.on('SIGINT', shutdown);
process.on('SIGTERM', shutdown);
