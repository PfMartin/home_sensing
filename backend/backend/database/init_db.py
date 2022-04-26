# https://www.digitalocean.com/community/tutorials/how-to-use-a-postgresql-database-in-a-flask-application

import os
from os.path import join, dirname
import psycopg2
from dotenv import load_dotenv

dotenv_path = join(dirname(__file__), 'database.env')
load_dotenv(dotenv_path)

conn = psycopg2.connect(
    host="localhost",
    database="home_sensing",
    user=os.environ.get("DB_USERNAME"),
    password=os.environ.get("DB_PASSWORD"),
)

cur = conn.cursor()

# cur.execute('DROP TABLE IF EXISTS temperature;')
cur.execute('CREATE TABLE temperature (id serial PRIMARY KEY,'
                                        'value real,'
                                        'timestamp date DEFAULT CURRENT_TIMESTAMP,'
                                        'location varchar (50) NOT NULL);'
)

# cur.execute('DROP TABLE IF EXISTS humidity;')
cur.execute('CREATE TABLE humidity (id serial PRIMARY KEY,'
                                        'value real,'
                                        'timestamp date DEFAULT CURRENT_TIMESTAMP,'
                                        'location varchar (50) NOT NULL);'
)

conn.commit()

cur.close()
conn.close()
