#!/usr/bin/env python3
"""
🎮 MINECRAFT BEDROCK LOGO PATCHER
Herramienta para parchear logos en librerías .so de Minecraft Bedrock
Funciona en Termux (Android) y Linux
"""

import sys
import struct
from PIL import Image
import io

def find_png_signatures(data):
    """Busca todas las firmas PNG en el archivo binario"""
    signatures = []
    png_sig = b'\x89PNG\r\n\x1a\n'
    
    pos = 0
    while True:
        pos = data.find(png_sig, pos)
        if pos == -1:
            break
        signatures.append(pos)
        pos += 1
    
    return signatures

def extract_png(data, offset):
    """Extrae una imagen PNG desde un offset específico"""
    pos = offset
    while pos < len(data) - 8:
        if data[pos:pos+4] == b'IEND':
            png_end = pos + 12
            return data[offset:png_end]
        pos += 1
    return None

def get_png_dimensions(png_data):
    """Obtiene las dimensiones de una imagen PNG"""
    try:
        img = Image.open(io.BytesIO(png_data))
        return img.size
    except:
        return None

def create_transparent_png(width, height):
    """Crea una imagen PNG transparente del tamaño especificado"""
    img = Image.new('RGBA', (width, height), (0, 0, 0, 0))
    png_bytes = io.BytesIO()
    img.save(png_bytes, format='PNG')
    return png_bytes.getvalue()

def resize_image(image_path, target_width, target_height):
    """Redimensiona una imagen a las dimensiones objetivo"""
    img = Image.open(image_path)
    img.thumbnail((target_width, target_height), Image.Resampling.LANCZOS)
    
    new_img = Image.new('RGBA', (target_width, target_height), (0, 0, 0, 0))
    offset = ((target_width - img.width) // 2, (target_height - img.height) // 2)
    new_img.paste(img, offset, img if img.mode == 'RGBA' else None)
    
    png_bytes = io.BytesIO()
    new_img.save(png_bytes, format='PNG', optimize=True)
    return png_bytes.getvalue()

def patch_library(so_file, new_png_data, output_file):
    """Parchea la librería con la nueva imagen PNG"""
    with open(so_file, 'rb') as f:
        data = bytearray(f.read())
    
    signatures = find_png_signatures(bytes(data))
    print(f"[*] Se encontraron {len(signatures)} imágenes PNG")
    
    if not signatures:
        print("[-] No se encontraron imágenes PNG en el archivo")
        return False
    
    pngs_info = []
    for i, offset in enumerate(signatures):
        png_data = extract_png(bytes(data), offset)
        if png_data:
            dims = get_png_dimensions(png_data)
            if dims:
                size_kb = len(png_data) / 1024
                pngs_info.append({
                    'index': i,
                    'offset': offset,
                    'size': len(png_data),
                    'dims': dims,
                    'size_kb': size_kb
                })
                print(f"[PNG {i}] Offset: 0x{offset:x} | Tamaño: {size_kb:.1f}KB | Dimensiones: {dims[0]}x{dims[1]}")
    
    if not pngs_info:
        print("[-] No se pudieron extraer imágenes PNG válidas")
        return False
    
    logo_info = max(pngs_info, key=lambda x: x['size'])
    logo_index = logo_info['index']
    logo_offset = logo_info['offset']
    logo_size = logo_info['size']
    logo_dims = logo_info['dims']
    
    print(f"\n[+] Logo identificado: PNG {logo_index}")
    print(f"    Offset: 0x{logo_offset:x}")
    print(f"    Tamaño: {logo_info['size_kb']:.1f}KB")
    print(f"    Dimensiones: {logo_dims[0]}x{logo_dims[1]}")
    
    if len(new_png_data) > logo_size:
        print(f"\n[!] ADVERTENCIA: La nueva imagen ({len(new_png_data)/1024:.1f}KB) es más grande que el original ({logo_info['size_kb']:.1f}KB)")
        print(f"    Se rellenará con bytes nulos (0x00)")
    
    print(f"\n[*] Reemplazando logo en offset 0x{logo_offset:x}...")
    
    for i, byte in enumerate(new_png_data):
        if i < logo_size:
            data[logo_offset + i] = byte
    
    if len(new_png_data) < logo_size:
        for i in range(len(new_png_data), logo_size):
            data[logo_offset + i] = 0x00
    
    with open(output_file, 'wb') as f:
        f.write(data)
    
    print(f"[+] ¡Archivo parchado exitosamente!")
    print(f"[+] Guardado en: {output_file}")
    return True

def main():
    if len(sys.argv) < 3:
        print("=" * 60)
        print("🎮 MINECRAFT BEDROCK LOGO PATCHER")
        print("=" * 60)
        print("\nUso:")
        print("  python3 patcher.py <librería.so> transparent <salida.so>")
        print("  python3 patcher.py <librería.so> <imagen.png> <salida.so>")
        print("\nEjemplos:")
        print("  python3 patcher.py libminecraftpe.so transparent libminecraftpe_patched.so")
        print("  python3 patcher.py libminecraftpe.so mi_logo.png libminecraftpe_patched.so")
        sys.exit(1)
    
    so_file = sys.argv[1]
    mode = sys.argv[2]
    output_file = sys.argv[3] if len(sys.argv) > 3 else "libminecraftpe_patched.so"
    
    print("=" * 60)
    print("🎮 MINECRAFT BEDROCK LOGO PATCHER")
    print("=" * 60)
    
    print(f"\n[*] Leyendo librería: {so_file}")
    try:
        with open(so_file, 'rb') as f:
            so_data = f.read()
        print(f"[+] Tamaño: {len(so_data) / (1024*1024):.1f}MB")
    except Exception as e:
        print(f"[-] Error al leer el archivo: {e}")
        return False
    
    if not so_data.startswith(b'\x7fELF'):
        print("[-] ADVERTENCIA: Este no parece ser un archivo ELF válido")
    
    if mode.lower() == 'transparent':
        print("\n[*] Modo: Crear logo TRANSPARENTE")
        
        signatures = find_png_signatures(so_data)
        if not signatures:
            print("[-] No se encontraron imágenes PNG")
            return False
        
        pngs_info = []
        for offset in signatures:
            png_data = extract_png(so_data, offset)
            if png_data:
                dims = get_png_dimensions(png_data)
                if dims:
                    pngs_info.append({'offset': offset, 'size': len(png_data), 'dims': dims})
        
        if not pngs_info:
            print("[-] No se pudieron extraer imágenes PNG válidas")
            return False
        
        logo_info = max(pngs_info, key=lambda x: x['size'])
        logo_dims = logo_info['dims']
        
        print(f"[*] Dimensiones del logo: {logo_dims[0]}x{logo_dims[1]}")
        new_png_data = create_transparent_png(logo_dims[0], logo_dims[1])
        print(f"[+] PNG transparente creado: {len(new_png_data)/1024:.1f}KB")
    else:
        image_path = mode
        print(f"\n[*] Modo: Usar imagen personalizada")
        print(f"[*] Imagen: {image_path}")
        
        signatures = find_png_signatures(so_data)
        if not signatures:
            print("[-] No se encontraron imágenes PNG")
            return False
        
        pngs_info = []
        for offset in signatures:
            png_data = extract_png(so_data, offset)
            if png_data:
                dims = get_png_dimensions(png_data)
                if dims:
                    pngs_info.append({'offset': offset, 'size': len(png_data), 'dims': dims})
        
        if not pngs_info:
            print("[-] No se pudieron extraer imágenes PNG válidas")
            return False
        
        logo_info = max(pngs_info, key=lambda x: x['size'])
        logo_dims = logo_info['dims']
        
        print(f"[*] Dimensiones del logo: {logo_dims[0]}x{logo_dims[1]}")
        print(f"[*] Redimensionando imagen...")
        
        try:
            new_png_data = resize_image(image_path, logo_dims[0], logo_dims[1])
            print(f"[+] Imagen redimensionada: {len(new_png_data)/1024:.1f}KB")
        except Exception as e:
            print(f"[-] Error al procesar la imagen: {e}")
            return False
    
    return patch_library(so_file, new_png_data, output_file)

if __name__ == '__main__':
    success = main()
    sys.exit(0 if success else 1)
