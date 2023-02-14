\c solarmeter

--
-- daily continuous aggregate
--
DROP MATERIALIZED VIEW IF EXISTS cagg_daily CASCADE;

CREATE MATERIALIZED VIEW cagg_daily
WITH (timescaledb.continuous, timescaledb.materialized_only=true) 
AS
SELECT
  time_bucket('1 day', time) AS bucket_1d,
  sensor_id,
  plan_id,
  last(total_energy, time) - first(total_energy, time) AS energy_1d,
  first(total_energy, time) AS total,
  avg(grid_power) AS power_avg,
  max(grid_power) AS power_max
FROM live
GROUP BY bucket_1d, plan_id, sensor_id
WITH NO DATA;

-- refresh policy
SELECT add_continuous_aggregate_policy('cagg_daily',
  start_offset => INTERVAL '3 days',
  end_offset => INTERVAL '1 hour',
  schedule_interval => '1 day');

--
-- average power
--
DROP MATERIALIZED VIEW IF EXISTS cagg_power CASCADE;

CREATE MATERIALIZED VIEW cagg_power
WITH (timescaledb.continuous, timescaledb.materialized_only=true)
AS
SELECT
  time_bucket('5 minutes', time) as bucket_5m,
  avg(grid_power) as power_5m,
  avg(power_p1) as power_p1,
  avg(power_p2) as power_p2
FROM live
GROUP BY bucket_5m           
WITH NO DATA;

-- refresh policy
SELECT add_continuous_aggregate_policy('cagg_power',
  start_offset => INTERVAL '3 hours',
  end_offset => INTERVAL '1 minute',
  schedule_interval => '5 minutes');

-- retention policy
SELECT add_retention_policy('cagg_power', INTERVAL '3 days');

-- grant
GRANT SELECT ON TABLE cagg_power TO grafana;
