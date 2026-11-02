#pragma once

#include <algorithm>
#include <cstddef>
#include <iomanip>
#include <istream>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>

// Funcoes de entrada compartilhadas. Todas as leituras usam getline.
namespace util {
inline std::string trim(const std::string& texto) {
    const auto inicio = texto.find_first_not_of(" \t\r\n");
    if (inicio == std::string::npos) return "";
    return texto.substr(inicio, texto.find_last_not_of(" \t\r\n") - inicio + 1);
}

inline std::string minuscula(std::string texto) {
    for (char& c : texto) {
        if (c >= 'A' && c <= 'Z') c = static_cast<char>(c - 'A' + 'a');
    }
    return texto;
}

inline std::string validarTexto(const std::string& texto, std::size_t maximo = 120) {
    const auto valor = trim(texto);
    if (valor.empty() || valor.size() > maximo)
        throw std::invalid_argument("Texto obrigatorio, com ate " + std::to_string(maximo) + " bytes.");
    for (unsigned char c : valor) {
        if (c < 32 || c == 127)
            throw std::invalid_argument("Texto nao pode conter caracteres de controle.");
    }
    return valor;
}

inline std::string validarTelefone(const std::string& texto) {
    const auto telefone = validarTexto(texto, 32);
    int digitos = 0;
    for (char c : telefone) {
        if (c >= '0' && c <= '9') ++digitos;
        else if (c != ' ' && c != '+' && c != '-' && c != '(' && c != ')')
            throw std::invalid_argument("Telefone aceita apenas numeros, espacos, +, -, ( e ).");
    }
    if (digitos < 8 || digitos > 15)
        throw std::invalid_argument("Telefone deve ter entre 8 e 15 digitos.");
    return telefone;
}

inline void validarMensalidade(long long valor) {
    if (valor < 1 || valor > 100000000LL)
        throw std::invalid_argument("Mensalidade permitida: de R$ 0,01 a R$ 1000000,00.");
}

inline std::string dinheiro(long long centavos) {
    std::ostringstream texto;
    texto << "R$ " << centavos / 100 << ',' << std::setfill('0')
          << std::setw(2) << centavos % 100;
    return texto.str();
}

inline std::string lerLinha(const std::string& prompt, std::istream& entrada,
                            std::ostream& saida) {
    saida << prompt;
    std::string linha;
    if (!std::getline(entrada, linha)) {
        if (entrada.eof()) throw std::ios_base::failure("Fim da entrada");
        throw std::runtime_error("Falha na leitura da entrada.");
    }
    return trim(linha);
}

inline bool converterInteiro(const std::string& texto, int& valor) {
    try {
        std::size_t posicao = 0;
        const int convertido = std::stoi(texto, &posicao);
        if (posicao != texto.size()) return false;
        valor = convertido;
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

inline bool converterCentavos(std::string texto, long long& valor) {
    std::replace(texto.begin(), texto.end(), ',', '.');
    const auto separador = texto.find('.');
    const auto reais = texto.substr(0, separador);
    auto centavos = separador == std::string::npos ? "" : texto.substr(separador + 1);
    const auto digitos = [](const std::string& campo) {
        return std::all_of(campo.begin(), campo.end(), [](char c) { return c >= '0' && c <= '9'; });
    };
    if (reais.empty() || reais.size() > 8 || !digitos(reais) ||
        centavos.size() > 2 || !digitos(centavos) ||
        (separador != std::string::npos && centavos.empty())) return false;
    while (centavos.size() < 2) centavos += '0';
    const long long convertido = std::stoll(reais) * 100 + std::stoll(centavos);
    if (convertido < 1 || convertido > 100000000LL) return false;
    valor = convertido;
    return true;
}

inline std::string lerTexto(const std::string& prompt, std::istream& entrada,
                             std::ostream& saida, std::size_t maximo = 120) {
    while (true) {
        const auto linha = lerLinha(prompt, entrada, saida);
        try { return validarTexto(linha, maximo); }
        catch (const std::invalid_argument& erro) { saida << erro.what() << '\n'; }
    }
}

inline std::string lerTelefone(const std::string& prompt, std::istream& entrada,
                                std::ostream& saida) {
    while (true) {
        const auto linha = lerLinha(prompt, entrada, saida);
        try { return validarTelefone(linha); }
        catch (const std::invalid_argument& erro) { saida << erro.what() << '\n'; }
    }
}

inline int lerInteiro(const std::string& prompt, int minimo, int maximo,
                      std::istream& entrada, std::ostream& saida) {
    while (true) {
        const auto linha = lerLinha(prompt, entrada, saida);
        int valor = 0;
        if (converterInteiro(linha, valor) && valor >= minimo && valor <= maximo) return valor;
        saida << "Digite um inteiro entre " << minimo << " e " << maximo << ".\n";
    }
}

inline long long lerCentavos(const std::string& prompt, std::istream& entrada,
                             std::ostream& saida) {
    while (true) {
        const auto linha = lerLinha(prompt, entrada, saida);
        long long valor = 0;
        if (converterCentavos(linha, valor)) return valor;
        saida << "Use de 0,01 a 1000000,00, sem separador de milhar.\n";
    }
}

inline std::string lerTextoOpcional(const std::string& prompt, const std::string& atual,
                                    std::istream& entrada, std::ostream& saida,
                                    std::size_t maximo = 120) {
    while (true) {
        const auto linha = lerLinha(prompt + " [" + atual + "]: ", entrada, saida);
        if (linha.empty()) return atual;
        try { return validarTexto(linha, maximo); }
        catch (const std::invalid_argument& erro) { saida << erro.what() << '\n'; }
    }
}

inline std::string lerTelefoneOpcional(const std::string& prompt, const std::string& atual,
                                       std::istream& entrada, std::ostream& saida) {
    while (true) {
        const auto linha = lerLinha(prompt + " [" + atual + "]: ", entrada, saida);
        if (linha.empty()) return atual;
        try { return validarTelefone(linha); }
        catch (const std::invalid_argument& erro) { saida << erro.what() << '\n'; }
    }
}

inline long long lerCentavosOpcional(const std::string& prompt, long long atual,
                                     std::istream& entrada, std::ostream& saida) {
    while (true) {
        const auto linha = lerLinha(prompt + " [" + dinheiro(atual) + "]: ", entrada, saida);
        if (linha.empty()) return atual;
        long long valor = 0;
        if (converterCentavos(linha, valor)) return valor;
        saida << "Use de 0,01 a 1000000,00, sem separador de milhar.\n";
    }
}

inline int lerInteiroOpcional(const std::string& prompt, int atual, int minimo, int maximo,
                             std::istream& entrada, std::ostream& saida) {
    while (true) {
        const auto linha = lerLinha(prompt + " [" + std::to_string(atual) + "]: ", entrada, saida);
        if (linha.empty()) return atual;
        int valor = 0;
        if (converterInteiro(linha, valor) && valor >= minimo && valor <= maximo) return valor;
        saida << "Digite um inteiro entre " << minimo << " e " << maximo << ".\n";
    }
}
} // namespace util
