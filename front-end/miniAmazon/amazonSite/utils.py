import socket

serverName = 'vcm-30576.vm.duke.edu'
serverPort = 6969

def construct_order_message(user, product_quantities):
    message = f'order\nuser_id:{user.id}\nuser_name:{user.username}\n'
    message = f'dest_x:{100}\ndest_y:{100}\n'
    for id_name_quantity in product_quantities:
        message += f'product_id:{id_name_quantity[0]}\nproduct_name:{id_name_quantity[1]}\nquantity:{id_name_quantity[2]}\n'
    message += "\n"

    return message

def try_place_order(user, product_quantity):
    try:
        message = construct_order_message(user=user, product_quantities=product_quantity)
        print(message)
        s = socket.socket(socket.AF_INET,socket.SOCK_STREAM)
        s.connect((serverName, serverPort))
        s.sendall(message.encode())
        
        s.close()
        return "success"
    except socket.error as e:
        print(f'Socket error: {e}')
        return f'Socket error: {e}'
    except Exception as ex:
        print(f'Error: {ex}')
        return f'Error: {ex}'