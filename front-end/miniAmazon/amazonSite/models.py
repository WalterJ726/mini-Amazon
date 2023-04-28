from django.db import models
from django.contrib.auth.models import User

class user_ups(models.Model):
    user_id = models.IntegerField(primary_key=True)
    ups_id = models.IntegerField(null=True, blank=True)

class Warehouse(models.Model):
    warehouse_id = models.AutoField(primary_key=True, default=0)
    address_x = models.IntegerField(null=True)
    address_y = models.IntegerField(null=True)

    def __str__(self) -> str:
        return f"({self.warehouse_id}, {self.address_x}, {self.address_y})"

class Product(models.Model):
    product_id = models.AutoField(primary_key=True, default=0)
    name = models.CharField(max_length=200, null=True)
    description = models.CharField(max_length=1000, null=True)

    def __str__(self) -> str:
        return f"({self.product_id}, {self.name}, {self.description})"


class Order(models.Model):
    order_num = models.IntegerField()
    product = models.ForeignKey("Product",null=True, on_delete=models.SET_NULL)
    user = models.ForeignKey(User, null=True, on_delete=models.SET_NULL)
    quantity = models.IntegerField()
    package_id = models.IntegerField(null=True, blank=True)
    create_time = models.TimeField(null=True, blank=True)

    class Meta:
        constraints = [
            models.UniqueConstraint(fields=['order_num', 'product'], name='order_product_pk'),
        ]


class Package(models.Model):
    package_id = models.AutoField(primary_key=True, default=0)
    owner = models.ForeignKey(User, null=True, on_delete=models.SET_NULL)
    warehouse_id = models.IntegerField()
    dest_x = models.IntegerField()
    dest_y = models.IntegerField()
    pack_time = models.TimeField(null=True, blank=True)
    ups_id = models.IntegerField(null=True, blank=True) # TODO: add foreign key
    truck_id = models.IntegerField(null=True, blank=True)
    package_status = models.CharField(max_length=100)

class Inventory(models.Model):
    warehouse = models.ForeignKey("Warehouse", on_delete=models.CASCADE)
    product = models.ForeignKey("Product", on_delete=models.CASCADE)
    quantity = models.PositiveIntegerField()

    class Meta:
        constraints = [
            models.UniqueConstraint(fields=['warehouse', 'product'], name='warehouse_product_pk'),
        ]