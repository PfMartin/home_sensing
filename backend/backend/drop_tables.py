from database_client import DatabaseClient

database_client = DatabaseClient()

database_client.connect()
database_client.drop_table("temperature")
database_client.drop_table("humidity")
database_client.drop_table("test")
database_client.disconnect()
