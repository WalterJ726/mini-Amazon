from django.urls import path
from . import views

app_name = 'amazonSite'
urlpatterns = [
    path('', views.login, name='login'),
    path('register/', views.register, name='register'),
    path('logout/', views.logout, name='logout'),
    path('dashboard/', views.dashboard, name='dashboard'),
    path('profile/', views.userProfile, name='profile'),
    path('orders/', views.orders, name='orders'),
    path('cart/', views.cart, name='cart'),
    path('inventory/', views.inventory, name='inventory'),
    path('shopping_mall/', views.shopping_mall, name='shopping_mall'),

]