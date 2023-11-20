#pragma once

template <typename TKey, typename TValue>
inline void MapToArrays(const TMap<TKey, TValue>& map, TArray<TKey>& keys, TArray<TValue>& values)
{
	keys.Empty();
	values.Empty();

	for (auto entry : map)
	{
		keys.Add(entry.Key);
		values.Add(entry.Value);
	}
}

template <typename TKey, typename TValue>
inline void ArraysToMap(const TArray<TKey>& keys, const TArray<TValue>& values, TMap<TKey, TValue>& map)
{
	map.Empty();

	for (typename TArray<TKey>::SizeType x = 0; x < keys.Num() && x < values.Num(); x++)
	{
		map[keys[x]] = values[x];
	}
}
