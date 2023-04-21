"""miniAmazon URL Configuration

The `urlpatterns` list routes URLs to views. For more information please see:
    https://docs.djangoproject.com/en/4.1/topics/http/urls/
Examples:
Function views
    1. Add an import:  from my_app import views
    2. Add a URL to urlpatterns:  path('', views.home, name='home')
Class-based views
    1. Add an import:  from other_app.views import Home
    2. Add a URL to urlpatterns:  path('', Home.as_view(), name='home')
Including another URLconf
    1. Import the include() function: from django.urls import include, path
    2. Add a URL to urlpatterns:  path('blog/', include('blog.urls'))
"""
from django.contrib import admin
from django.urls import path, include

urlpatterns = [
    path('admin/', admin.site.urls),
    path('amazonSite/', include('amazonSite.urls')),
    #path('amazonSite/dashboard/', views.dashboard),
    # path('login/', views.login, name='login'),
    #path('ride', views.login, name='login'),
    # path('ride/driver_reg/', views.driver_reg, name='driver_reg'),
    # path('ride/driverHome/', views.driverHome, name='driverHome'),
    # path('ride/d_search_ride/', views.driver_search_ride),
    # path('driver_ride/', views.driver_ride),
    # path('ride/driver_rideEdit/', views.driver_ride),
    # path('ride/user_rideInfo/', views.user_rideInfo),
]