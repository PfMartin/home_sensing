import os
from os.path import join, dirname
import psycopg2
from psycopg2 import sql
from dotenv import load_dotenv

dotenv_path = join(dirname(__file__), "database.env")
load_dotenv(dotenv_path)


class DatabaseClient:
    def __init__(self, host, username, password, db_name):
        self.host = host
        self.username = username
        self.password = password
        self.db_name = db_name

    def connect(self):
        self.connection = psycopg2.connect(
            host=self.host,
            database=self.db_name,
            user=self.username,
            password=self.password,
        )

    def disconnect(self):
        self.connection.close()

    def create_table(self, table_name):
        cur = self.connection.cursor()

        cur.execute(
            sql.SQL(
                "CREATE TABLE {table} (id serial PRIMARY KEY, value real, timestamp date DEFAULT CURRENT_TIMESTAMP, location varchar (50) NOT NULL)"
            ).format(table=sql.Identifier(table_name))
        )

        self.connection.commit()
        cur.close()

    def drop_table(self, table_name):
        cur = self.connection.cursor()

        cur.execute(
            sql.SQL("DROP TABLE IF EXISTS {table}").format(
                table=sql.Identifier(table_name)
            )
        )

        self.connection.commit()
        cur.close()
