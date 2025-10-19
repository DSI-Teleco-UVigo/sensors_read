const statusEl = document.getElementById('status');
const latestMetaEl = document.getElementById('latestMeta');
const latestValuesEl = document.getElementById('latestValues');

const palette = [
  '#ff6b6b',
  '#4dabf7',
  '#38d9a9',
  '#f59f00',
  '#845ef7',
  '#ffa94d',
  '#20c997',
  '#e599f7',
  '#66d9e8',
  '#ffe066',
  '#94d82d',
  '#ff922b',
];

function numberOrNull(value) {
  if (value === null || value === undefined) {
    return null;
  }
  const numeric = Number(value);
  return Number.isFinite(numeric) ? numeric : null;
}

function formatTimestamp(value) {
  if (!value) {
    return 'n/a';
  }
  const date = new Date(Number(value) * 1000);
  return Number.isNaN(date.getTime()) ? 'n/a' : date.toISOString();
}

function createLineChart(canvasId, { series, maxPoints = 160, emptyMessage }) {
  const canvas = document.getElementById(canvasId);
  const ctx = canvas.getContext('2d');
  const history = [];
  let logicalWidth = canvas.width;
  let logicalHeight = canvas.height;
  let deviceRatio = window.devicePixelRatio || 1;

  function resize() {
    const width = canvas.clientWidth;
    const height = canvas.clientHeight;
    if (width === 0 || height === 0) {
      return;
    }

    deviceRatio = window.devicePixelRatio || 1;
    logicalWidth = width;
    logicalHeight = height;
    canvas.width = Math.floor(width * deviceRatio);
    canvas.height = Math.floor(height * deviceRatio);
    ctx.setTransform(deviceRatio, 0, 0, deviceRatio, 0, 0);
    draw();
  }

  function draw() {
    ctx.clearRect(0, 0, logicalWidth, logicalHeight);

    if (history.length === 0) {
      ctx.fillStyle = '#99a2b1';
      ctx.font = '16px sans-serif';
      ctx.fillText(emptyMessage || 'Waiting for samples…', 20, 40);
      return;
    }

    const padding = 60;
    const plotWidth = Math.max(0, logicalWidth - padding * 2);
    const plotHeight = Math.max(0, logicalHeight - padding * 2);

    const values = [];
    series.forEach((serie) => {
      history.forEach((entry) => {
        const value = entry.values[serie.id];
        if (typeof value === 'number' && Number.isFinite(value)) {
          values.push(value);
        }
      });
    });

    if (values.length === 0) {
      ctx.fillStyle = '#99a2b1';
      ctx.font = '16px sans-serif';
      ctx.fillText('No numeric values in the stream yet.', 20, 40);
      return;
    }

    const minValue = Math.min(...values);
    const maxValue = Math.max(...values);
    const valueSpan = maxValue - minValue;
    const verticalPadding = valueSpan === 0 ? 1 : valueSpan * 0.1;
    const yMin = minValue - verticalPadding;
    const yMax = maxValue + verticalPadding;

    const mapX = (index) => {
      if (history.length === 1) {
        return padding;
      }
      const ratio = index / (history.length - 1);
      return padding + ratio * plotWidth;
    };

    const mapY = (value) => {
      const clamped = Math.min(Math.max(value, yMin), yMax);
      const ratio = (clamped - yMin) / (yMax - yMin || 1);
      return padding + (1 - ratio) * plotHeight;
    };

    ctx.strokeStyle = '#2f3446';
    ctx.lineWidth = 1;
    ctx.beginPath();
    ctx.moveTo(padding, padding);
    ctx.lineTo(padding, logicalHeight - padding);
    ctx.lineTo(logicalWidth - padding, logicalHeight - padding);
    ctx.stroke();

    const gridLines = 5;
    ctx.font = '12px sans-serif';
    ctx.fillStyle = '#99a2b1';
    ctx.textAlign = 'right';
    ctx.textBaseline = 'middle';

    for (let i = 0; i <= gridLines; i += 1) {
      const ratio = i / gridLines;
      const y = padding + ratio * plotHeight;
      const value = (yMax - ratio * (yMax - yMin)).toFixed(2);

      ctx.strokeStyle = '#1f2534';
      ctx.beginPath();
      ctx.moveTo(padding, y);
      ctx.lineTo(logicalWidth - padding, y);
      ctx.stroke();

      ctx.fillText(value, padding - 12, y);
    }

    ctx.textAlign = 'left';
    ctx.textBaseline = 'middle';
    let legendX = padding;
    const legendY = padding - 30;
    ctx.font = '12px sans-serif';
    series.forEach((serie) => {
      ctx.fillStyle = serie.color;
      ctx.fillRect(legendX, legendY, 12, 12);
      ctx.fillStyle = '#cbd5e1';
      ctx.fillText(serie.label, legendX + 18, legendY + 6);
      legendX += ctx.measureText(serie.label).width + 36;
    });

    series.forEach((serie) => {
      ctx.strokeStyle = serie.color;
      ctx.lineWidth = 2;
      ctx.beginPath();
      let penUp = true;
      history.forEach((entry, index) => {
        const value = entry.values[serie.id];
        if (typeof value !== 'number' || !Number.isFinite(value)) {
          penUp = true;
          return;
        }
        const x = mapX(index);
        const y = mapY(value);
        if (penUp) {
          ctx.moveTo(x, y);
          penUp = false;
        } else {
          ctx.lineTo(x, y);
        }
      });
      ctx.stroke();
    });
  }

  window.addEventListener('resize', resize);
  requestAnimationFrame(resize);

  return {
    addSample: (values) => {
      if (!values || typeof values !== 'object') {
        return;
      }
      let hasNumeric = false;
      series.forEach((serie) => {
        const value = values[serie.id];
        if (typeof value === 'number' && Number.isFinite(value)) {
          hasNumeric = true;
        }
      });

      if (!hasNumeric) {
        return;
      }

      history.push({ values });
      if (history.length > maxPoints) {
        history.shift();
      }
      draw();
    },
    redraw: draw,
  };
}

function updateLatest(payload) {
  if (!payload) {
    return;
  }

  const { readingTimestamp, receivedAt } = payload;
  latestMetaEl.textContent = `Reading timestamp: ${formatTimestamp(readingTimestamp)} | Received at: ${new Date(receivedAt).toLocaleTimeString()}`;
  latestValuesEl.textContent = JSON.stringify(payload.sensors, null, 2);
}

const imuChart = createLineChart('imuChart', {
  series: [
    { id: 'mpu_ax', label: 'MPU9250 ax', color: palette[0] },
    { id: 'mpu_ay', label: 'MPU9250 ay', color: palette[1] },
    { id: 'mpu_az', label: 'MPU9250 az', color: palette[2] },
    { id: 'lsm_ax', label: 'LSM9DS1 ax', color: palette[3] },
    { id: 'lsm_ay', label: 'LSM9DS1 ay', color: palette[4] },
    { id: 'lsm_az', label: 'LSM9DS1 az', color: palette[5] },
  ],
  maxPoints: 150,
  emptyMessage: 'Waiting for IMU samples…',
});

const rcChart = createLineChart('rcChart', {
  series: [
    { id: 'roll', label: 'Roll', color: palette[0] },
    { id: 'pitch', label: 'Pitch', color: palette[1] },
    { id: 'throttle', label: 'Throttle', color: palette[2] },
    { id: 'yaw', label: 'Yaw', color: palette[3] },
  ],
  maxPoints: 180,
  emptyMessage: 'Waiting for RCInput samples…',
});

const gpsChart = createLineChart('gpsChart', {
  series: [
    { id: 'latitude', label: 'Latitude', color: palette[0] },
    { id: 'longitude', label: 'Longitude', color: palette[1] },
    { id: 'height', label: 'Height', color: palette[2] },
  ],
  maxPoints: 200,
  emptyMessage: 'Waiting for GPS samples…',
});

const envChart = createLineChart('envChart', {
  series: [
    { id: 'temperature', label: 'Temperature', color: palette[0] },
    { id: 'pressure', label: 'Pressure', color: palette[1] },
    { id: 'a0', label: 'ADC a0', color: palette[2] },
    { id: 'a1', label: 'ADC a1', color: palette[3] },
    { id: 'a2', label: 'ADC a2', color: palette[4] },
    { id: 'a3', label: 'ADC a3', color: palette[5] },
    { id: 'a4', label: 'ADC a4', color: palette[6] },
    { id: 'a5', label: 'ADC a5', color: palette[7] },
  ],
  maxPoints: 200,
  emptyMessage: 'Waiting for barometer/ADC samples…',
});

const charts = [
  {
    handler: imuChart,
    extractor: (payload) => {
      const mpu = payload.sensors?.['IMU/MPU9250'] || {};
      const lsm = payload.sensors?.['IMU/LSM9DS1'] || {};
      return {
        mpu_ax: numberOrNull(mpu.ax),
        mpu_ay: numberOrNull(mpu.ay),
        mpu_az: numberOrNull(mpu.az),
        lsm_ax: numberOrNull(lsm.ax),
        lsm_ay: numberOrNull(lsm.ay),
        lsm_az: numberOrNull(lsm.az),
      };
    },
  },
  {
    handler: rcChart,
    extractor: (payload) => {
      const rc = payload.sensors?.RCInput || {};
      return {
        roll: numberOrNull(rc.roll),
        pitch: numberOrNull(rc.pitch),
        throttle: numberOrNull(rc.throttle),
        yaw: numberOrNull(rc.yaw),
      };
    },
  },
  {
    handler: gpsChart,
    extractor: (payload) => {
      const gps = payload.sensors?.GPS || {};
      return {
        latitude: numberOrNull(gps.lat),
        longitude: numberOrNull(gps.lon),
        height: numberOrNull(gps.height),
      };
    },
  },
  {
    handler: envChart,
    extractor: (payload) => {
      const barometer = payload.sensors?.Barometer || {};
      const adc = payload.sensors?.ADC || {};
      return {
        temperature: numberOrNull(barometer.temperature),
        pressure: numberOrNull(barometer.pressure),
        a0: numberOrNull(adc.a0),
        a1: numberOrNull(adc.a1),
        a2: numberOrNull(adc.a2),
        a3: numberOrNull(adc.a3),
        a4: numberOrNull(adc.a4),
        a5: numberOrNull(adc.a5),
      };
    },
  },
];

function handlePayload(payload) {
  if (!payload) {
    return;
  }

  updateLatest(payload);
  charts.forEach(({ handler, extractor }) => {
    try {
      handler.addSample(extractor(payload));
    } catch (error) {
      console.error('Failed to process chart data', error);
    }
  });
}

function setupStream() {
  const source = new EventSource('/stream');

  source.onopen = () => {
    statusEl.textContent = 'Connected';
    statusEl.classList.remove('disconnected');
  };

  source.onmessage = (event) => {
    try {
      const payload = JSON.parse(event.data);
      handlePayload(payload);
    } catch (error) {
      console.error('Failed to parse payload', error);
    }
  };

  source.onerror = () => {
    statusEl.textContent = 'Connection lost. Retrying…';
    statusEl.classList.add('disconnected');
  };
}

setupStream();
