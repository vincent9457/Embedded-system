"""fcu URL Configuration

The `urlpatterns` list routes URLs to views. For more information please see:
    https://docs.djangoproject.com/en/3.2/topics/http/urls/
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
from django.urls import re_path as url
from app import views

urlpatterns = [
    url(r'^admin/', admin.site.urls),
    url(r'^G000/', views.G000),
    url(r'^G001/', views.G001),
    url(r'^G002/', views.G002),
    url(r'^G005/', views.G005),
    url(r'^G004/', views.G004),
    url(r'^upload/', views.upload_sensor_data),
    url(r'^upload_temp/$', views.get_temp_data),
    url(r'^upload_sensor_db/$', views.upload_sensor_to_new_db),
    url(r'^upload_rfid/$', views.upload_rfid_checkin),
    url(r'^upload_env/$', views.upload_environment_data),
]
