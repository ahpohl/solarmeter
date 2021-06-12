\c solarmeter
CREATE EXTENSION IF NOT EXISTS timescaledb;

DROP TABLE IF EXISTS "live" CASCADE;
DROP TABLE IF EXISTS "sensors" CASCADE;

CREATE TABLE "sensors" (
  id SERIAL PRIMARY KEY,
  serial VARCHAR(50),
  mfg_date VARCHAR(50),
  part_no VARCHAR(50),
  firmware VARCHAR(50),
  grid VARCHAR(50),
  inverter VARCHAR(50),
  type VARCHAR(50),
  transformer BOOLEAN
);

CREATE TABLE "live" (
  time TIMESTAMPTZ NOT NULL,
  sensor_id INTEGER NOT NULL,
  voltage_1 DOUBLE PRECISION,
  current_1 DOUBLE PRECISION,
  power_1 DOUBLE PRECISION,
  voltage_2 DOUBLE PRECISION,
  current_2 DOUBLE PRECISION,
  power_2 DOUBLE PRECISION,
  grid_voltage DOUBLE PRECISION,
  grid_current DOUBLE PRECISION,
  grid_power DOUBLE PRECISION,
  grid_frequency DOUBLE PRECISION,
  inverter_temp DOUBLE PRECISION,
  booster_temp DOUBLE PRECISION,
  total_energy DOUBLE PRECISION,
  vbulk DOUBLE PRECISION,
  riso DOUBLE PRECISION,
  CONSTRAINT sensor_id FOREIGN KEY (sensor_id) REFERENCES sensors (id)
);

SELECT create_hypertable('live', 'time');

INSERT INTO sensors(id, serial, mfg_date, part_no, firmware, grid, inverter, type, transformer) VALUES 
(1, '126014', 'May 2010', '3G79', 'C.0.2.2', 'Germany-VDE0126', 'PVI-4.2-OUTD', 'PV', false);

GRANT INSERT, SELECT ON TABLE live TO nodejs;
GRANT SELECT ON TABLE live TO grafana;

GRANT SELECT ON TABLE sensors TO nodejs;
GRANT SELECT ON TABLE sensors TO grafana;
