# Academia — cadastro de alunos em C++

Projeto de **Linguagem de Programação I — 2026.2**, tema **33. Academia**. O sistema mantém cadastros de alunos de musculação e de treinamento funcional, consulta nomes, altera e remove registros e apresenta um relatório das mensalidades previstas.

A organização usa o projeto de Livraria enviado como referência: uma classe por arquivo `.h`, uma classe gerenciadora das operações e uma `main.cpp` pequena. O domínio e os dados foram adaptados para uma academia. A aplicação e os testes são escritos em **C++17**, sem Python ou bibliotecas externas.

## Compilar e executar

É necessário um compilador C++17, como GCC, e GNU Make:

```sh
make
./academia
```

Também é possível compilar diretamente:

```sh
g++ -std=c++17 -Wall -Wextra -Wpedantic main.cpp -o academia
./academia
```

No Windows, esses comandos podem ser usados no WSL. O arquivo padrão é `alunos.txt`, relativo ao diretório em que o programa for executado. Para usar outro arquivo:

```sh
./academia /tmp/minha-academia.txt
```

Execute os testes em C++ com:

```sh
make test
```

Os **17 testes em C++ passaram** na verificação do projeto. Eles cobrem CRUD, pesquisa e nomes repetidos, validação de dados, relatório, persistência das duas modalidades, interrupção da entrada e preservação dos arquivos em falhas.

`make clean` remove os executáveis e os arquivos de compilação; os cadastros não são apagados.

## Menu e dados

| Opção | Operação |
| --- | --- |
| 1 | Inserir aluno de musculação ou treinamento funcional |
| 2 | Listar todos os alunos |
| 3 | Exibir um aluno pesquisando pelo nome |
| 4 | Alterar um aluno pesquisando pelo nome |
| 5 | Remover um aluno pesquisando pelo nome |
| 6 | Exibir relatório |
| 7 | Salvar e sair |

Todo aluno possui nome, telefone, mensalidade e data de matrícula. O aluno de musculação possui objetivo e instrutor; o de treinamento funcional possui quantidade de aulas semanais e nível. A data de matrícula é preenchida automaticamente e permanece a mesma na alteração.

Os nomes aceitam espaços. A pesquisa recebe o nome completo ou apenas o início: por exemplo, `Ana` pode encontrar `Ana Clara` e `Ana Paula`. Quando existem vários resultados, o programa mostra seus índices atuais no vetor, que começam em **0**, e solicita a escolha de um deles. `-1` cancela a seleção. A comparação ignora diferenças entre maiúsculas e minúsculas ASCII; letras acentuadas devem ser digitadas com a mesma capitalização.

Na alteração, **Enter mantém o valor atual**. A modalidade do aluno é mantida; para trocar de modalidade, remova o registro e cadastre o aluno novamente. A remoção exige confirmação.

Os textos são obrigatórios e aceitam até 120 bytes, incluindo espaços. Telefones aceitam até 32 bytes, devem ter de 8 a 15 dígitos e podem conter espaços, parênteses, hífen e sinal de mais. A mensalidade aceita valores de **R$ 0,01 a R$ 1.000.000,00**, sem separador de milhar; use vírgula ou ponto para separar centavos. O valor é guardado como um inteiro em centavos. O treinamento funcional aceita de 1 a 7 aulas semanais. As datas são validadas, incluindo anos bissextos. O cadastro tem limite de 100.000 alunos.

O relatório apresenta quantidade de alunos, soma das mensalidades, média e resumo por modalidade. São valores **previstos conforme os cadastros**: o sistema não registra pagamentos recebidos nem controla inadimplência.

## Requisitos do trabalho

| Requisito | Onde está implementado |
| --- | --- |
| CRUD e menu com sete opções | `main.cpp` e `GerenciadorAcademia.h` |
| Pesquisa por nome completo ou prefixo, incluindo espaços | `GerenciadorAcademia::pesquisar` |
| Método de pesquisa que retorna índice e é reutilizado | `pesquisar`, chamado por `exibirUm`, `alterar` e `remover` |
| Desambiguação de nomes repetidos | Exibição dos candidatos e escolha pelo índice |
| Cada classe em um `.h` separado | `Data.h`, `Aluno.h`, `AlunoMusculacao.h`, `AlunoFuncional.h`, `GerenciadorAcademia.h` |
| Quatro atributos privados no objeto principal | Nome, telefone, mensalidade e data de matrícula em `Aluno` |
| Composição com uma classe própria | `Aluno` contém um objeto `Data` |
| Uma superclasse e duas subclasses | `Aluno`, `AlunoMusculacao` e `AlunoFuncional` |
| Atributos privados nas subclasses | Objetivo/instrutor e aulas semanais/nível |
| Vetor com polimorfismo | `std::vector<std::unique_ptr<Aluno>>` |
| Sobrescrita de métodos | `getTipo`, `exibir`, `editar`, `serializar` e `clonar` nas subclasses |
| Classe que gerencia o CRUD | `GerenciadorAcademia` |
| Relatório diferente da listagem | Contagens e valores por modalidade, com mensalidade média geral |
| Execução até a opção de saída | Laço do menu em `main.cpp` |
| Arquivo único para dados de subclasses diferentes | `alunos.txt`, com identificador de modalidade em cada registro |
| Carga na entrada e gravação na saída | Construtor do gerenciador e `salvar` ao encerrar |
| Operações de cadastro em memória | Todas atuam no vetor, não diretamente no arquivo |
| Diagrama UML | [PDF](docs/diagrama_classes.pdf), [SVG](docs/diagrama_classes.svg) e [fonte Mermaid](docs/diagrama_classes.mermaid) |

`std::unique_ptr` é responsável pela liberação de cada aluno quando o registro sai do vetor ou o gerenciador é destruído. O destrutor virtual de `Aluno` permite destruir corretamente objetos das subclasses. Na edição, uma cópia polimórfica é alterada antes de substituir o registro: uma entrada interrompida não deixa a alteração pela metade.

## Persistência

O arquivo começa com `ACADEMIA_V1`. Cada linha seguinte identifica a modalidade e guarda os campos do aluno. Textos usam aspas com escape e datas são gravadas no formato `AAAA-MM-DD`. Os dois tipos de aluno compartilham o mesmo arquivo.

Se o arquivo não existir, o programa inicia com uma lista vazia. Se ele for inválido, a aplicação informa o erro e preserva o conteúdo original. A gravação prepara um arquivo temporário antes de substituir o arquivo definitivo. O arquivo só é carregado ao abrir a aplicação e gravado ao encerrá-la; inserções, consultas, alterações e remoções usam o vetor em memória.

A opção 7 e o fim normal da entrada do terminal salvam os cadastros completos. Encerrar o processo à força pode perder alterações ainda em memória. Evite executar duas instâncias usando o mesmo arquivo. Para fazer backup, encerre o programa e copie `alunos.txt`.

## Entrega

- [Diagrama UML em PDF](docs/diagrama_classes.pdf).
- [Diagrama UML em SVG](docs/diagrama_classes.svg) e fontes [Mermaid](docs/diagrama_classes.mermaid) / [Graphviz](docs/diagrama_classes.dot).
- [Roteiro da apresentação, até 10 minutos](docs/apresentacao.md).
- [Declaração de uso de IA](docs/uso-de-ia.md).

IA utilizada como apoio na elaboração e revisão do projeto, incluindo código, testes e documentação.
