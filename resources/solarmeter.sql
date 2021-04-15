\c solarmeter
CREATE EXTENSION IF NOT EXISTS timescaledb;

DROP TABLE IF EXISTS "input" CASCADE;
DROP TABLE IF EXISTS "grid" CASCADE;
DROP TABLE IF EXISTS "stats" CASCADE;
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

CREATE TABLE "input" (
  time TIMESTAMPTZ NOT NULL,
  sensor_id INTEGER NOT NULL,
  voltage_1 DOUBLE PRECISION,
  current_1 DOUBLE PRECISION,
  power_1 DOUBLE PRECISION,
  voltage_2 DOUBLE PRECISION,
  current_2 DOUBLE PRECISION,
  power_2 DOUBLE PRECISION,
  CONSTRAINT sensor_id FOREIGN KEY (sensor_id) REFERENCES sensors (id)
);

CREATE TABLE "grid" (
  time TIMESTAMPTZ NOT NULL,
  sensor_id INTEGER NOT NULL,
  voltage DOUBLE PRECISION,
  current DOUBLE PRECISION,
  power DOUBLE PRECISION,
  frequency DOUBLE PRECISION,
  CONSTRAINT sensor_id FOREIGN KEY (sensor_id) REFERENCES sensors (id)
);

CREATE TABLE "stats" (
  time TIMESTAMPTZ NOT NULL,
  sensor_id INTEGER NOT NULL,
  temp_inverter DOUBLE PRECISION,
  temp_booster DOUBLE PRECISION,
  energy_total DOUBLE PRECISION,
  CONSTRAINT sensor_id FOREIGN KEY (sensor_id) REFERENCES sensors (id)
);

SELECT create_hypertable('input', 'time');
SELECT create_hypertable('grid', 'time');
SELECT create_hypertable('stats', 'time');

INSERT INTO sensors(id, serial, mfg_date, part_no, firmware, grid, inverter, type, transformer) VALUES 
(1, '126014', 'May 2010', '3G79', 'C.0.2.2', 'Germany-VDE0126', 'PVI-4.2-OUTD', 'PV', false);

GRANT INSERT, SELECT ON TABLE input TO nodejs;
GRANT SELECT ON TABLE input TO grafana;

GRANT INSERT, SELECT ON TABLE grid TO nodejs;
GRANT SELECT ON TABLE grid TO grafana;

GRANT INSERT, SELECT ON TABLE stats TO nodejs;
GRANT SELECT ON TABLE stats TO grafana;

GRANT SELECT ON TABLE sensors TO nodejs;
GRANT SELECT ON TABLE sensors TO grafana;
