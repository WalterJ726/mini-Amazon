from django.shortcuts import get_object_or_404, render, redirect
from django.contrib.auth.models import User
from django.contrib.auth.decorators import login_required
from django.contrib import messages, auth
from .models import Inventory, Product
from .utils import try_place_order
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
        product_quantities = []
        for field_name, field_value in request.POST.items():
            if field_name.endswith('_quantity') and len(field_value) != 0:
                product_id = int(field_name.split('_')[0])
                product_name = field_name.split('_')[1]
                quantity = int(field_value)
                if quantity > 0:
                    product_quantities.append((product_id, product_name, quantity))

        print(product_quantities)
        feedback = try_place_order(request.user, product_quantities)

        if feedback == "success" :
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

