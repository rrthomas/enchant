These are generic tests to test the providers. They should work on any provider.
When a provider does not support a particular function, those tests will just pass.

Some tests rely on the provider having specific language support:
fr_FR

The tests try to clean up after themselves but some words may be added to
the personal dictionary if the provider does not provide a way to remove
words from the personal dictionary.