def Settings( **kwargs ):
    return {
        'flags': [
            '-x','c++',  # O el lenguaje que estés usando, por ejemplo 'c', 'c++', 'objective-c', etc.
            '-std=c++17',  # O el estándar que estés utilizando
            '-I', '/opt/homebrew/Cellar/sfml/3.0.0_1/include',  
            '-I', './build/Debug/_deps/imgui-src',  
            '-I', './build/Debug/_deps/imgui-sfml-src',  
        ],
    }

