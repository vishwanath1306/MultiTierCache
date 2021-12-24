from distutils.core import setup, Extension

def main():

    extension_1 = Extension("simulator",
                            include_dirs=['/usr/local/include/', '/usr/include/glib-2.0', '/usr/lib/x86_64-linux-gnu/glib-2.0/include'],
                            library_dirs=['/usr/local/lib/'],
                            libraries=['libCacheSim', 'glib-2.0', 'm', 'dl'],
                            sources=["python_binding.c"]
                            )
    setup(name="simulator",
          version="1.0.0",
          description="Python interface for Simulator C library",
          author="Vishwanath Seshagiri",
          author_email="vseshag@emory.edu",
          ext_modules=[extension_1])

if __name__ == "__main__":
    main()