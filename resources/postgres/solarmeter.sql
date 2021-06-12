\c solarmeter

DROP MATERIALIZED VIEW IF EXISTS "monthly_view" CASCADE;
DROP MATERIALIZED VIEW IF EXISTS "daily_view" CASCADE;
DROP MATERIALIZED VIEW IF EXISTS "cagg_power" CASCADE;
DROP MATERIALIZED VIEW IF EXISTS "cagg_daily" CASCADE;
DROP TABLE IF EXISTS "archive" CASCADE;
DROP TABLE IF EXISTS "live" CASCADE;
DROP TABLE IF EXISTS "sensors" CASCADE;

CREATE EXTENSION IF NOT EXISTS timescaledb;

CREATE TABLE "sensors" (
  id SERIAL PRIMARY KEY,
  serial_num VARCHAR(50),
  part_num VARCHAR(50),
  mfg_date VARCHAR(50),
  firmware VARCHAR(50),
  inverter_type VARCHAR(50),
  grid_standard VARCHAR(50)
);

CREATE TABLE "live" (
  time TIMESTAMPTZ NOT NULL,
  sensor_id INTEGER NOT NULL,
  total_energy DOUBLE PRECISION,
  voltage_p1 DOUBLE PRECISION,
  current_p1 DOUBLE PRECISION,
  power_p1 DOUBLE PRECISION,
  voltage_p2 DOUBLE PRECISION,
  current_p2 DOUBLE PRECISION,
  power_p2 DOUBLE PRECISION,
  grid_voltage DOUBLE PRECISION,
  grid_current DOUBLE PRECISION,
  grid_power DOUBLE PRECISION,
  frequency DOUBLE PRECISION,
  efficiency DOUBLE PRECISION,
  inverter_temp DOUBLE PRECISION,
  booster_temp DOUBLE PRECISION,
  r_iso DOUBLE PRECISION,
  payment DOUBLE PRECISION,
  CONSTRAINT sensor_id FOREIGN KEY (sensor_id) REFERENCES sensors (id)
);

SELECT create_hypertable('live', 'time');
SELECT add_retention_policy('live', INTERVAL '7 days');

INSERT INTO sensors(id, serial_num, part_num, mfg_date, firmware, inverter_type, grid_standard) VALUES
(1, '126014', '-3G79-', 'Year 10 Week 20', 'C.0.2.2', 'Aurora 4.2kW new', 'VDE0126');

GRANT INSERT, SELECT ON TABLE live TO nodejs;
GRANT SELECT ON TABLE live TO grafana;

GRANT SELECT ON TABLE sensors TO nodejs;
GRANT SELECT ON TABLE sensors TO grafana;
