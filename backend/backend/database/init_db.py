# https://www.digitalocean.com/community/tutorials/how-to-use-a-postgresql-database-in-a-flask-application

import os
import psycopg2

conn = psycopg2.connect(
    host="localhost",
    database="home_sensing",
    user=os.environ["DB_USERNAME"],
    password=os.environ["DB_PASSWORD"],
)

cur = conn.cursor()
