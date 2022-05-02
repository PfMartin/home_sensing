from DatabaseClient import DatabaseClient

database_client = DatabaseClient()

database_client.connect()
database_client.create_table("temperature")
database_client.create_table("humidity")
database_client.disconnect()
