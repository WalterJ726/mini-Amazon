from django.shortcuts import get_object_or_404, render, redirect
from django.contrib.auth.models import User
from django.contrib.auth.decorators import login_required
from django.contrib import messages, auth
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