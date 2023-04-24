from django.shortcuts import get_object_or_404, render, redirect
from django.contrib.auth.models import User
from django.contrib.auth.decorators import login_required
from django.contrib import messages, auth
from .models import Inventory
import socket
# Create your views here.

@login_required
def dashboard(request):
    user = request.user
    #user_ex = UserEx.objects.get_or_create(user=request.user)
    context = {}
    context = {'user_name':user.username}
    return render(request, 'amazonSite/dashboard.html', context)

def login(request):
    if request.method=="POST":
        username=request.POST['username']
        password=request.POST['password']
        user = auth.authenticate(username=username, password=password)
        if user is not None:
            auth.login(request, user)
            return redirect('/amazonSite/dashboard/')
        else:
            messages.info(request, 'user is not exist or wrong password!')
            return render(request, 'amazonSite/login.html', locals())
    else:
        return render(request, 'amazonSite/login.html')

        
def register(request):
    if request.method=="POST":
        first_name = request.POST['first_name']
        last_name = request.POST['last_name']
        username = request.POST['username']
        password = request.POST['password']
        password2 = request.POST['password2']
        email = request.POST['email']

        if password == password2:
            if User.objects.filter(username=username).exists():
                messages.info(request, 'user exist!')
                return render(request, 'amazonSite/register.html', locals())
            else:
                if User.objects.filter(email=email).exists():
                    messages.info(request, 'email exist!')
                    return render(request, 'amazonSite/register.html', locals())
                else: 
                    user = User.objects.create_user(username=username, email=email, password=password, first_name=first_name,last_name=last_name)
                    user.save()
                    #user_ex = UserEx.objects.create(user=user)
                    #/user_ex.save()
                    messages.success(request, 'Registration successful!')
                    return render(request, 'amazonSite/login.html', locals())

        else:
            messages.info(request, 'password not same!')
            return render(request, 'amazonSite/register.html', locals())
    else:
        return render(request, 'amazonSite/register.html')


@login_required  
def logout(request):
    auth.logout(request)
    return redirect('/amazonSite/')

def sendToServer(message):
    ip_port = ('vcm-30576.vm.duke.edu', 6969)
    s = socket.socket()
    s.connect(ip_port)
    s.sendall(message.encode())
    s.close()



@login_required
def shopping_mall(request):
    if request.method=="POST":
        print(request.POST)
        user = request.user
        tosend = "user_id:" + str(user.id) + "\n" + "user_name:" + str(user.username) + "\n"
        for field_name, field_value in request.POST.items():
            if field_name.endswith('_quantity') and len(field_value) != 0 :
                parts = field_name.split('_')
                product_id = int(parts[0])
                product_name = parts[1]
                quantity = int(field_value)
                if quantity > 0:
                    tosend += "product_id:" + str(product_id) + "\n" + "product_name:" + product_name + "\n" + "quantity:" + str(quantity) + "\n"
        
        print(tosend)
        sendToServer(tosend)
        
        messages.success(request, 'Submit successfully!')
        return render(request, 'amazonSite/shopping_mall.html', locals())

    return render(request, 'amazonSite/shopping_mall.html')

@login_required
def inventory(request):
    inventories = Inventory.objects.all()
    context = {'inventories': inventories}
    print(context)
    return render(request, 'amazonSite/inventory.html', context)

