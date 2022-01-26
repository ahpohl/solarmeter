\c solarmeter

-- create daily view with data from archive and cagg_daily
DROP MATERIALIZED VIEW IF EXISTS daily_view CASCADE;

CREATE MATERIALIZED VIEW daily_view
AS
SELECT
  bucket_1d AS time,
  energy_1d AS energy,
  energy_1d * payment AS credit,
  total,
  payment
FROM archive JOIN plan ON archive.plan_id = plan.id
GROUP BY bucket_1d, energy_1d, total, payment
UNION
SELECT
  bucket_1d AS time,
  energy_1d AS energy,
  energy_1d * payment AS credit,
  total,
  payment
FROM cagg_daily JOIN plan ON cagg_daily.plan_id = plan.id
--WHERE bucket_1d > TIMESTAMP WITH TIME ZONE '2021-10-07 01:00:00+01'
GROUP BY bucket_1d, energy_1d, total, payment
ORDER BY time;

-- index
CREATE UNIQUE INDEX daily_idx ON daily_view (time);

-- grant
GRANT SELECT ON TABLE daily_view TO grafana;

--
-- create monthly view
--
CREATE MATERIALIZED VIEW monthly_view
AS
SELECT
  timescaledb_experimental.time_bucket_ng('1 month', time) AS time,
  sum(energy) AS energy,
  sum(credit) AS credit,
  first(total, time) AS total
FROM daily_view
GROUP BY timescaledb_experimental.time_bucket_ng('1 month', time)
ORDER BY time;

-- index
CREATE UNIQUE INDEX monthly_idx ON monthly_view (time);

-- grant
GRANT SELECT ON TABLE monthly_view TO grafana;

--
-- create yearly view
--
CREATE MATERIALIZED VIEW yearly_view
AS
SELECT
  timescaledb_experimental.time_bucket_ng('1 year', time) AS time,
  sum(energy) AS energy,
  sum(credit) AS credit,
  first(total, time) AS total
FROM daily_view
GROUP BY timescaledb_experimental.time_bucket_ng('1 year', time)
ORDER BY time;

-- index
CREATE UNIQUE INDEX yearly_idx ON yearly_view (time);

-- grant
GRANT SELECT ON TABLE yearly_view TO grafana;
