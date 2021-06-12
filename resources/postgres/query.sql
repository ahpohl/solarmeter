\c solarmeter
INSERT INTO live(
  time, sensor_id,
  voltage_1, current_1, power_1,
  voltage_2, current_2, power_2,
  grid_voltage, grid_current, grid_power, grid_frequency,
  inverter_temp, booster_temp, total_energy,
  vbulk, riso
)
VALUES(
  '2021-04-15T20:20:04.000Z', 1,
  304.506622, 2.844574, 866.191528, 
  302.308868, 2.888076, 873.091003,
  233.888977, 7.155008, 1687.964111, 49.997002,
  43.141850, 39.591328, 46353.912,
  371.053436, 12.943206
);
