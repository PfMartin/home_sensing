import os
from os.path import join, dirname
import psycopg2
from psycopg2 import sql
from dotenv import load_dotenv

from datetime import datetime


class DatabaseClient:
    def __init__(self):
        dotenv_path = join(dirname(__file__), "..", "..", "Database.env")
        load_dotenv(dotenv_path)

        self.host = os.environ.get("DB_HOST")
        self.username = os.environ.get("DB_USERNAME")
        self.password = os.environ.get("DB_PASSWORD")
        self.db_name = os.environ.get("DB_DATABASE")

    def connect(self):
        self.connection = psycopg2.connect(
            host=self.host,
            database=self.db_name,
            user=self.username,
            password=self.password,
        )

    def disconnect(self):
        self.connection.close()

    def list_tables(self):
        cur = self.connection.cursor()
        cur.execute(
            "SELECT table_name FROM information_schema.tables WHERE table_schema = 'public'"
        )

        tables = []

        for table in cur.fetchall():
            tables.append(table[0])

        cur.close()

        return tables

    def create_table(self, table_name):
        cur = self.connection.cursor()

        cur.execute(
            sql.SQL(
                "CREATE TABLE {table} (id serial PRIMARY KEY, value real, timestamp timestamp DEFAULT CURRENT_TIMESTAMP, location varchar (50) NOT NULL)"
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

    def sync_tables(self, desired_tables):
        existing_tables = self.list_tables()

        for desired_table in desired_tables:
            if desired_table not in existing_tables:
                self.create_table(desired_table)

        for existing_table in existing_tables:
            if existing_table not in desired_tables:
                self.drop_table(existing_table)

    def insert_data(self, table_name, value, timestamp, location):
        cur = self.connection.cursor()

        cur.execute(
            sql.SQL(
                "INSERT INTO {table} (value, timestamp, location) VALUES (%s, %s, %s)"
            ).format(table=sql.Identifier(table_name)),
            [value, timestamp, location],
        )

        self.connection.commit()
        cur.close()


if __name__ == "__main__":
    database_client = DatabaseClient()
    database_client.connect()
    database_client.sync_tables(["temperature", "humidity"])
    database_client.disconnect()
