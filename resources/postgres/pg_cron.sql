\c pg_cron

CREATE EXTENSION IF NOT EXISTS pg_cron;

-- regularly purge the log
SELECT cron.schedule('purge_log', '13 0 * * *', $$DELETE
    FROM cron.job_run_details
    WHERE end_time < now() - INTERVAL '3 days'$$);

-- schedule updates
SELECT cron.schedule('solarmeter_daily', '2,17,32,47 * * * *', $$REFRESH
  MATERIALIZED VIEW CONCURRENTLY daily_view$$);

SELECT cron.schedule('solarmeter_monthly', '3,18,33,48 * * * *', $$REFRESH
  MATERIALIZED VIEW CONCURRENTLY monthly_view$$);

UPDATE cron.job SET database = 'solarmeter' WHERE 
  jobname = 'solarmeter_daily' OR jobname = 'solarmeter_monthly';
