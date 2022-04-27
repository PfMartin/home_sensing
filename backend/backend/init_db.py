# https://www.digitalocean.com/community/tutorials/how-to-use-a-postgresql-database-in-a-flask-application

import os
from os.path import join, dirname
import psycopg2
from dotenv import load_dotenv
from DatabaseClient import DatabaseClient

dotenv_path = join(dirname(__file__), "database.env")
load_dotenv(dotenv_path)

database_client = DatabaseClient(
    os.environ.get("DB_HOST"),
    os.environ.get("DB_USERNAME"),
    os.environ.get("DB_PASSWORD"),
    os.environ.get("DB_DATABASE"),
)

database_client.connect()
database_client.create_table('temperature')
database_client.create_table('humidity')
database_client.disconnect()
