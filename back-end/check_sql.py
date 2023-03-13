from turtle import end_fill
import mysql.connector as sql
# print(sql)
mydb = sql.connect(
    host = 'localhost',
    port="3307",
    user = 'root',
    password = '123456',
    database = 'IOT', 
    auth_plugin='mysql_native_password'
)
print("Connect success")
cursor = mydb.cursor(prepared = True)
cursor.execute("SELECT * FROM `log` ORDER BY `time` DESC LIMIT 10")
all_result = cursor.fetchall()
print(type(all_result))
# print(all_result)
my_dict = dict()
for record in all_result:
    print(record[0], record[1], record[2])
