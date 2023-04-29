import socket
from .models import Cart, Product

serverName = 'vcm-30576.vm.duke.edu'
serverPort = 6969

def construct_order_message(user, product_quantities, dest_x, dest_y):
    message = f'order\nuser_id:{user.id}\nuser_name:{user.username}\n'
    message += f'dest_x:{dest_x}\ndest_y:{dest_y}\n'
    for id_name_quantity in product_quantities:
        message += f'product_id:{id_name_quantity[0]}\nproduct_name:{id_name_quantity[1]}\nquantity:{id_name_quantity[2]}\n'
    message += "\n"

    return message

def try_place_order(user, product_quantity, dest_x, dest_y):
    try:
        message = construct_order_message(user=user, product_quantities=product_quantity, dest_x=dest_x, dest_y=dest_y)
        print(message)
        s = socket.socket(socket.AF_INET,socket.SOCK_STREAM)
        s.connect((serverName, serverPort))
        s.sendall(message.encode())
        text = s.recv(1024).decode()
        print("Order reponse is: " + text)
        s.close()
        return text
    except socket.error as e:
        print(f'Socket error: {e}')
        return f'Socket error: {e}'
    except Exception as ex:
        print(f'Error: {ex}')
        return f'Error: {ex}'
    
def construct_bind_message(user_ups, new_ups_id):
    message = f'bind\nuser_id:{user_ups.user_id}\nups_id:{new_ups_id}\n'
    message += "\n"
    return message

def try_bind_ups(user_ups, new_ups_id):
    try:
        message = construct_bind_message(user_ups=user_ups, new_ups_id=new_ups_id)
        print(message)
        s = socket.socket(socket.AF_INET,socket.SOCK_STREAM)
        s.connect((serverName, serverPort))
        s.sendall(message.encode())
        s.close()

    except socket.error as e:
        print(f'Socket error: {e}')
        return f'Socket error: {e}'
    except Exception as ex:
        print(f'Error: {ex}')
        return f'Error: {ex}'
    
def add_to_cart(user, product_quantity):
    for id_name_quantity in product_quantity:
        product = Product.objects.get(product_id=id_name_quantity[0])
        try:
            cart = Cart.objects.get(user=user, product=product)
            cart.quantity += id_name_quantity[2]
            cart.save()
        except Cart.DoesNotExist:
            cart = Cart(user=user, product=product, quantity=id_name_quantity[2])
            cart.save()