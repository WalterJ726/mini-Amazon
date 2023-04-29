from django.shortcuts import get_object_or_404, render, redirect
from django.contrib.auth.models import User
from django.contrib.auth.decorators import login_required
from django.contrib import messages, auth
from .models import Inventory, Product, user_ups, Order, Package
from django.db.models import Q
from .utils import try_place_order
from .utils import try_bind_ups
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
                    new_user_ups = user_ups(user_id=user.pk, ups_id=None) #create an object that is a new user with Null ups_id
                    new_user_ups.save() #insert it to user_ups table
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

@login_required
def shopping_mall(request):
    inventories = Inventory.objects.all()  # Fetch all inventories from the Inventory model
    product_stock = {}
    for inventory in inventories:
        product_id = inventory.product.product_id
        quantity = inventory.quantity
        if product_id not in product_stock:
            product = Product.objects.get(product_id=product_id)  # Fetch the related product
            product_stock[product_id] = {'name': product.name, 'description': product.description, 'stock': 0}
        product_stock[product_id]['stock'] += quantity

    if request.method == "POST":
        dest_x = request.POST['dest_x']
        dest_y = request.POST['dest_y']
        product_quantities = []
        for field_name, field_value in request.POST.items():
            if field_name.endswith('_quantity') and len(field_value) != 0:
                product_id = int(field_name.split('_')[0])
                product_name = field_name.split('_')[1]
                quantity = int(field_value)
                if quantity > 0:
                    product_quantities.append((product_id, product_name, quantity))

        print(product_quantities)
        feedback = try_place_order(request.user, product_quantities, dest_x=dest_x, dest_y=dest_y)

        if feedback == "Order placed successfully!" :
            messages.success(request, 'Order placed successfully!')
        else:
            messages.error(request, feedback)

        return render(request, 'amazonSite/shopping_mall.html', {'product_stock': product_stock})

    return render(request, 'amazonSite/shopping_mall.html', {'product_stock': product_stock})



@login_required
def inventory(request):
    inventories = Inventory.objects.all()
    warehouse_products_dict = {}  # Create an empty dictionary to store warehouse_id as keys and products as values
    for inventory in inventories:
        warehouse_id = inventory.warehouse.warehouse_id
        product = inventory.product
        quantity = inventory.quantity
        if warehouse_id not in warehouse_products_dict:
            warehouse_products_dict[warehouse_id] = []
        warehouse_products_dict[warehouse_id].append((product, quantity))

    # Sort the products list for each warehouse_id by product_id in ascending order
    for warehouse_id in warehouse_products_dict:
        warehouse_products_dict[warehouse_id].sort(key=lambda x: x[0].product_id)

    return render(request, 'amazonSite/inventory.html', {'warehouse_products_dict': warehouse_products_dict})


@login_required
def userProfile(request):   
    user = request.user
    curr_user_ups = user_ups.objects.filter(user_id = user.id).first()
    context = {'user_name':user.username, 'user_email':user.email,'last_name':user.last_name, 'first_name': user.first_name, 'ups_id': curr_user_ups.ups_id, 'bind_status': curr_user_ups.bind_status}
        
    if request.method == 'POST':
        password_old = request.POST.get('password1', '')   # use POST.get() to avoid CSRF attack
        password_new1 = request.POST.get('password2', '')
        password_new2 = request.POST.get('password3', '')
        email = request.POST.get('email', '')
        ups_id = request.POST.get('ups_id', None)

        if password_old != '' and (password_new1 == '' or password_new2 == ''):
            messages.info(request, 'Please input new password!')
            return render(request, 'amazonSite/profile.html', context)
        if password_old == '' and (password_new1 != '' or password_new2 != ''):
            messages.info(request, 'Please input old password!')
            return render(request, 'amazonSite/profile.html', context)
        if password_old != '' and password_new1 != '' and password_new2 != '':
            if auth.authenticate(username=request.user.username, password=password_old) is None:
                messages.info(request, 'Wrong user old password!')
                return render(request, 'amazonSite/profile.html', context)

            if password_new1 != password_new2:
                messages.info(request, 'Two password is not the same!')
                return render(request, 'amazonSite/profile.html', context)
            
            user.set_password(password_new1) # old password is correct and new passwords match
                 
            if email != '':    
                if User.objects.filter(Q(email=email) & ~Q(username=user.username)).exists():
                    messages.info(request, 'The email address is used, please use another one!')
                    return render(request, 'amazonSite/profile.html', context) 
                user.email = email
                context['user_email'] = email

            if ups_id != None:
                if user_ups.objects.filter(Q(ups_id=ups_id) & ~Q(user_id=user.id)).exists():
                    messages.info(request, 'The UPS ID is used, please use another one!')
                    return render(request, 'amazonSite/profile.html', context) 
                curr_user_ups.ups_id = ups_id
                context['ups_id'] = ups_id
                feedback = try_bind_ups(user_ups=curr_user_ups, new_ups_id=ups_id)
                if (feedback != None):
                    messages.info(request, feedback)

        if password_old == '' and password_new1 == '' and password_new2 == '':        
            if email != '':    
                if User.objects.filter(Q(email=email) & ~Q(username=user.username)).exists():
                    messages.info(request, 'The email address is used, please use another one!')
                    return render(request, 'amazonSite/profile.html', context) 
                user.email = email
                context['user_email'] = email

            if ups_id != None:
                if user_ups.objects.filter(Q(ups_id=ups_id) & ~Q(user_id=user.id)).exists():
                    messages.info(request, 'The UPS ID is used, please use another one!')
                    return render(request, 'amazonSite/profile.html', context) 
                curr_user_ups.ups_id = ups_id
                context['ups_id'] = ups_id
                feedback = try_bind_ups(user_ups=curr_user_ups, new_ups_id=ups_id)
                if (feedback != None):
                    messages.info(request, feedback)

        user.save()
        curr_user_ups.save()
        messages.info(request, 'Changes saved!')
        return render(request, 'amazonSite/profile.html', context)
        
    return render(request, 'amazonSite/profile.html', context)


@login_required
def orders(request):
    current_user = request.user
    orders = Order.objects.filter(user=current_user).order_by('-create_time')
    
    # create a dictionary to store orders and their associated packages
    order_packages_dict = {}
    
    # iterate through each order and add its packages to the dictionary
    for order in orders:
        key = order
        if key not in order_packages_dict:
            order_packages_dict[key] = []
        packages = Package.objects.filter(package_id = order.package_id)
        for package in packages:
            order_packages_dict[key].append(package)

    
    context = {
        'order_packages_dict': order_packages_dict
    }
    
    print(context)
    
    return render(request, 'amazonSite/orders.html', context)