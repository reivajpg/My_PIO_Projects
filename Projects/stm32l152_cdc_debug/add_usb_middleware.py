import os
from SCons.Script import Import

Import("env")

# Obtener el directorio donde está instalado el framework STM32CubeL1
platform = env.PioPlatform()
package_dir = platform.get_package_dir("framework-stm32cubel1")
project_src_dir = env.subst("$PROJECT_SRC_DIR")

if package_dir:
    # Definir las rutas a los archivos fuente del Middleware USB dentro del framework
    usb_core_src = os.path.join(package_dir, "Middlewares", "ST", "STM32_USB_Device_Library", "Core", "Src")
    usb_cdc_src = os.path.join(package_dir, "Middlewares", "ST", "STM32_USB_Device_Library", "Class", "CDC", "Src")

    # Agregar estos directorios al filtro de fuentes (SRC_FILTER)
    # Usamos +<ruta> para incluir los archivos
    # IMPORTANTE: Debemos incluir explícitamente "+<src/>" para que compile nuestro código.
    # También excluimos (-<...>) los templates para evitar conflictos de redefinición.
    env.Append(SRC_FILTER=[
        f"+<{project_src_dir}>",
        f"+<{usb_core_src}>",
        f"+<{usb_cdc_src}>",
        f"-<{usb_core_src}/usbd_conf_template.c>",
        f"-<{usb_core_src}/usbd_desc_template.c>",
        f"-<{usb_cdc_src}/usbd_cdc_if_template.c>"
    ])